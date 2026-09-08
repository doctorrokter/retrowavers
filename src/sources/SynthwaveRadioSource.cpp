/*
 * SynthwaveRadioSource.cpp - see SynthwaveRadioSource.hpp.
 */

#include "SynthwaveRadioSource.hpp"

#include <bb/data/JsonDataAccess>

#include <QNetworkRequest>
#include <QUrl>
#include <QTimer>
#include <QStringList>
#include <QDebug>

using namespace bb::data;

static const char* kStatusUrl = "https://stream.synthwaveradio.eu/api/nowplaying/synthwaveradio.eu";
static const char* kStreamUrl = "https://stream.synthwaveradio.eu/listen/synthwaveradio.eu/radio.mp3";
static const char* kStationId = "main";

static const int kPollMs = 30000;

SynthwaveRadioSource::SynthwaveRadioSource(QObject* parent) : ITrackSource(parent),
        m_network(new QNetworkAccessManager(this)), m_poll(new QTimer(this)), m_delivered(false) {
    m_poll->setInterval(kPollMs);
    bool res = QObject::connect(m_poll, SIGNAL(timeout()), this, SLOT(onPollTimeout()));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

SynthwaveRadioSource::~SynthwaveRadioSource() {
    m_poll->stop();
    m_network->deleteLater();
}

QString SynthwaveRadioSource::id() const { return "synthwaveradio"; }
QString SynthwaveRadioSource::name() const { return "SynthwaveRadio.eu"; }
QString SynthwaveRadioSource::kind() const { return "radio"; }

bool SynthwaveRadioSource::canList() const { return true; }
bool SynthwaveRadioSource::canDownload() const { return false; }

void SynthwaveRadioSource::reset() {
    m_delivered = false;
    m_activeTrackId = "";
    m_poll->stop();
}

void SynthwaveRadioSource::loadMore() {
    if (m_delivered) {
        emit tracksLoaded(QVariantList());
        return;
    }

    QVariantMap map;
    map["id"] = id() + ":" + kStationId;
    map["title"] = name();
    map["author"] = name();
    map["name"] = name();
    map["tags"] = QStringList() << "synthwave";
    map["duration"] = 0;
    map["streamUrl"] = kStreamUrl;
    map["artworkUrl"] = "";   // per-track art arrives with the first poll
    map["bArtworkUrl"] = "";
    map["favourite"] = false;
    map["filename"] = "";

    QVariantList tracks;
    tracks.append(map);

    m_delivered = true;
    qDebug() << "===>>> SynthwaveRadioSource#loadMore stations: 1" << endl;
    emit tracksLoaded(tracks);
}

void SynthwaveRadioSource::setActiveTrack(const QString& trackId) {
    if (!trackId.startsWith(id() + ":")) {
        m_activeTrackId = "";
        m_poll->stop();
        return;
    }

    if (m_activeTrackId == trackId) {
        return;
    }

    m_activeTrackId = trackId;
    requestNowPlaying();
    m_poll->start();
}

void SynthwaveRadioSource::onPollTimeout() {
    requestNowPlaying();
}

void SynthwaveRadioSource::requestNowPlaying() {
    if (m_activeTrackId.isEmpty()) {
        return;
    }

    QNetworkRequest req;
    req.setUrl(QUrl(kStatusUrl));

    QNetworkReply* reply = m_network->get(req);
    reply->setProperty("trackId", m_activeTrackId);
    bool res = QObject::connect(reply, SIGNAL(finished()), this, SLOT(onStatusReply()));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

void SynthwaveRadioSource::onStatusReply() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    QString trackId = reply->property("trackId").toString();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "===>>> SynthwaveRadioSource#onStatusReply error: " << reply->errorString() << endl;
        reply->deleteLater();
        return;
    }

    if (trackId != m_activeTrackId) {
        reply->deleteLater();
        return;
    }

    JsonDataAccess jda;
    QVariantMap root = jda.loadFromBuffer(reply->readAll()).toMap();

    QVariantMap song = root.value("now_playing").toMap().value("song").toMap();
    const QString artist = song.value("artist").toString();
    const QString title = song.value("title").toString();

    if (!title.isEmpty()) {
        QVariantMap fields;
        fields["title"] = artist.isEmpty() ? title : (artist + " " + QString(QChar(0x2013)) + " " + title);
        fields["author"] = artist;
        fields["name"] = title;

        const QString art = song.value("art").toString();
        if (!art.isEmpty()) {
            fields["artworkUrl"] = art;
        }

        qDebug() << "===>>> SynthwaveRadioSource: on air " << fields.value("title").toString() << endl;
        emit trackUpdated(trackId, fields);
    }

    reply->deleteLater();
}
