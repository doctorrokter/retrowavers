/*
 * SomaFmSource.cpp - see SomaFmSource.hpp.
 */

#include "SomaFmSource.hpp"

#include <bb/data/JsonDataAccess>

#include <QNetworkRequest>
#include <QUrl>
#include <QTimer>
#include <QXmlStreamReader>
#include <QStringList>
#include <QDebug>

using namespace bb::data;

static const char* kChannelsUrl = "https://somafm.com/channels.json";
static const char* kNowPlayingUrl = "https://somafm.com/songs/%1.xml";

// Plain http on purpose - see the note in the header about mm-renderer and TLS.
static const char* kStreamUrl = "http://ice1.somafm.com/%1-128-mp3";

// How often we ask what is on air. Tracks run three to six minutes, so this is
// frequent enough to look live without hammering somafm.com.
static const int kPollMs = 30000;

SomaFmSource::SomaFmSource(QObject* parent) : ITrackSource(parent),
        m_network(new QNetworkAccessManager(this)), m_poll(new QTimer(this)),
        m_inFlight(false), m_delivered(false) {
    m_poll->setInterval(kPollMs);
    bool res = QObject::connect(m_poll, SIGNAL(timeout()), this, SLOT(onPollTimeout()));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

SomaFmSource::~SomaFmSource() {
    m_poll->stop();
    m_network->deleteLater();
}

QString SomaFmSource::id() const { return "soma"; }
QString SomaFmSource::name() const { return "SomaFM"; }
QString SomaFmSource::kind() const { return "radio"; }

bool SomaFmSource::canList() const { return true; }
bool SomaFmSource::canDownload() const { return false; }

void SomaFmSource::reset() {
    m_delivered = false;
    m_activeTrackId = "";
    m_poll->stop();
}

void SomaFmSource::loadMore() {
    if (m_inFlight) {
        return;
    }

    if (m_delivered) {
        // The station list is complete after one request; there is no next page.
        // Answer anyway - the list view keeps its spinner up until we do.
        emit tracksLoaded(QVariantList());
        return;
    }

    m_inFlight = true;

    QNetworkRequest req;
    req.setUrl(QUrl(kChannelsUrl));

    qDebug() << "===>>> SomaFmSource#loadMore " << kChannelsUrl << endl;

    QNetworkReply* reply = m_network->get(req);
    bool res = QObject::connect(reply, SIGNAL(finished()), this, SLOT(onChannelsReply()));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

void SomaFmSource::onChannelsReply() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    m_inFlight = false;

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "===>>> SomaFmSource#onChannelsReply error: " << reply->errorString() << endl;
        emit loadFailed(reply->errorString());
        reply->deleteLater();
        return;
    }

    JsonDataAccess jda;
    QVariantMap response = jda.loadFromBuffer(reply->readAll()).toMap();
    QVariantList channels = response.value("channels").toList();

    QVariantList tracks;
    foreach(QVariant var, channels) {
        tracks.append(toTrackMap(var.toMap()));
    }

    m_delivered = true;
    qDebug() << "===>>> SomaFmSource#onChannelsReply stations: " << tracks.size() << endl;

    emit tracksLoaded(tracks);
    reply->deleteLater();
}

QVariantMap SomaFmSource::toTrackMap(const QVariantMap& channel) const {
    QVariantMap map;

    QString channelId = channel.value("id").toString();
    QString title = channel.value("title").toString();

    map["id"] = id() + ":" + channelId;
    map["title"] = title;
    map["author"] = name();
    map["name"] = title;

    // "electronic|specials" is SomaFM's own genre field; it doubles nicely as tags.
    map["tags"] = channel.value("genre").toString().split("|", QString::SkipEmptyParts);

    // A live stream has no length. QML formats this as 00:00 and the player simply
    // counts up, which is what a radio should do.
    map["duration"] = 0;

    map["streamUrl"] = QString(kStreamUrl).arg(channelId);
    map["artworkUrl"] = channel.value("xlimage").toString();
    map["bArtworkUrl"] = "";
    map["favourite"] = false;
    map["filename"] = "";

    return map;
}

QString SomaFmSource::channelIdOf(const QString& trackId) const {
    const QString prefix = id() + ":";
    return trackId.startsWith(prefix) ? trackId.mid(prefix.length()) : QString("");
}

void SomaFmSource::setActiveTrack(const QString& trackId) {
    QString channel = channelIdOf(trackId);
    if (channel.isEmpty()) {
        // Something else is playing (another source, or nothing) - stop polling.
        m_activeTrackId = "";
        m_poll->stop();
        return;
    }

    if (m_activeTrackId == trackId) {
        return;
    }

    m_activeTrackId = trackId;
    requestNowPlaying();   // don't wait a full interval to show the first song
    m_poll->start();
}

void SomaFmSource::onPollTimeout() {
    requestNowPlaying();
}

void SomaFmSource::requestNowPlaying() {
    QString channel = channelIdOf(m_activeTrackId);
    if (channel.isEmpty()) {
        return;
    }

    QNetworkRequest req;
    req.setUrl(QUrl(QString(kNowPlayingUrl).arg(channel)));

    QNetworkReply* reply = m_network->get(req);
    reply->setProperty("trackId", m_activeTrackId);
    bool res = QObject::connect(reply, SIGNAL(finished()), this, SLOT(onNowPlayingReply()));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

void SomaFmSource::onNowPlayingReply() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    QString trackId = reply->property("trackId").toString();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "===>>> SomaFmSource#onNowPlayingReply error: " << reply->errorString() << endl;
        reply->deleteLater();
        return;
    }

    if (trackId != m_activeTrackId) {
        reply->deleteLater();   // the station changed while this was in flight
        return;
    }

    // The feed lists the last ~20 songs, newest first; the first one is on air.
    QString artist;
    QString song;
    QString albumArt;

    QXmlStreamReader xml(reply->readAll());
    bool inSong = false;
    while (!xml.atEnd() && song.isEmpty()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == "song") {
                inSong = true;
            } else if (inSong) {
                if (xml.name() == "title") {
                    song = xml.readElementText();
                } else if (xml.name() == "artist") {
                    artist = xml.readElementText();
                } else if (xml.name() == "albumart") {
                    albumArt = xml.readElementText();
                }
            }
        }
    }

    if (!song.isEmpty()) {
        QVariantMap fields;
        // Same "artist - track" shape the catalogue produces, so the player and the
        // now-playing overlay need no special case for radio.
        fields["title"] = artist.isEmpty() ? song : (artist + " " + QString(QChar(0x2013)) + " " + song);
        fields["author"] = artist;
        fields["name"] = song;
        if (!albumArt.isEmpty()) {
            fields["artworkUrl"] = albumArt;
        }

        qDebug() << "===>>> SomaFmSource: on air " << fields.value("title").toString() << endl;
        emit trackUpdated(trackId, fields);
    }

    reply->deleteLater();
}
