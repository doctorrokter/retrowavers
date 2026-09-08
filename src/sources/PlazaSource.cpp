/*
 * PlazaSource.cpp - see PlazaSource.hpp.
 */

#include "PlazaSource.hpp"

#include <bb/data/JsonDataAccess>

#include <QNetworkRequest>
#include <QUrl>
#include <QTimer>
#include <QStringList>
#include <QDebug>

using namespace bb::data;

static const char* kStatusUrl = "https://api.plaza.one/status";
static const char* kStreamUrl = "http://radio.plaza.one/mp3";
static const char* kStationId = "plaza";

static const int kPollMs = 30000;

PlazaSource::PlazaSource(QObject* parent) : ITrackSource(parent),
        m_network(new QNetworkAccessManager(this)), m_poll(new QTimer(this)), m_delivered(false) {
    m_poll->setInterval(kPollMs);
    bool res = QObject::connect(m_poll, SIGNAL(timeout()), this, SLOT(onPollTimeout()));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

PlazaSource::~PlazaSource() {
    m_poll->stop();
    m_network->deleteLater();
}

QString PlazaSource::id() const { return "plaza"; }
QString PlazaSource::name() const { return "Nightwave Plaza"; }
QString PlazaSource::kind() const { return "radio"; }

bool PlazaSource::canList() const { return true; }
bool PlazaSource::canDownload() const { return false; }

void PlazaSource::reset() {
    m_delivered = false;
    m_activeTrackId = "";
    m_poll->stop();
}

void PlazaSource::loadMore() {
    if (m_delivered) {
        emit tracksLoaded(QVariantList());
        return;
    }

    QVariantMap map;
    map["id"] = id() + ":" + kStationId;
    map["title"] = name();
    map["author"] = name();
    map["name"] = name();
    map["tags"] = QStringList() << "vaporwave";
    map["duration"] = 0;
    map["streamUrl"] = kStreamUrl;
    map["artworkUrl"] = "";   // filled in per track once we know what is playing
    map["bArtworkUrl"] = "";
    map["favourite"] = false;
    map["filename"] = "";

    QVariantList tracks;
    tracks.append(map);

    m_delivered = true;
    qDebug() << "===>>> PlazaSource#loadMore stations: 1" << endl;
    emit tracksLoaded(tracks);
}

void PlazaSource::setActiveTrack(const QString& trackId) {
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

void PlazaSource::onPollTimeout() {
    requestNowPlaying();
}

void PlazaSource::requestNowPlaying() {
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

void PlazaSource::onStatusReply() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    QString trackId = reply->property("trackId").toString();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "===>>> PlazaSource#onStatusReply error: " << reply->errorString() << endl;
        reply->deleteLater();
        return;
    }

    if (trackId != m_activeTrackId) {
        reply->deleteLater();
        return;
    }

    JsonDataAccess jda;
    QVariantMap song = jda.loadFromBuffer(reply->readAll()).toMap().value("song").toMap();

    const QString artist = song.value("artist").toString();
    const QString title = song.value("title").toString();

    if (!title.isEmpty()) {
        QVariantMap fields;
        fields["title"] = artist.isEmpty() ? title : (artist + " " + QString(QChar(0x2013)) + " " + title);
        fields["author"] = artist;
        fields["name"] = title;

        // The one radio service that gives us real cover art per track.
        const QString artwork = song.value("artwork_src").toString();
        if (!artwork.isEmpty()) {
            fields["artworkUrl"] = artwork;
        }

        qDebug() << "===>>> PlazaSource: on air " << fields.value("title").toString() << endl;
        emit trackUpdated(trackId, fields);
    }

    reply->deleteLater();
}
