/*
 * RetrowaveOneSource.cpp - see RetrowaveOneSource.hpp.
 */

#include "RetrowaveOneSource.hpp"

#include <bb/data/JsonDataAccess>

#include <QNetworkRequest>
#include <QUrl>
#include <QTimer>
#include <QStringList>
#include <QDebug>

using namespace bb::data;

// Baked-in address - see the caveat in the header.
static const char* kStatusUrl = "http://77.108.192.88:8000/status-json.xsl";
static const char* kStreamUrl = "http://77.108.192.88:8000/stream";
static const char* kMount = "stream";

static const int kPollMs = 30000;

RetrowaveOneSource::RetrowaveOneSource(QObject* parent) : ITrackSource(parent),
        m_network(new QNetworkAccessManager(this)), m_poll(new QTimer(this)), m_delivered(false) {
    m_poll->setInterval(kPollMs);
    bool res = QObject::connect(m_poll, SIGNAL(timeout()), this, SLOT(onPollTimeout()));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

RetrowaveOneSource::~RetrowaveOneSource() {
    m_poll->stop();
    m_network->deleteLater();
}

QString RetrowaveOneSource::id() const { return "retrowaveone"; }
QString RetrowaveOneSource::name() const { return "Retrowave.One"; }
QString RetrowaveOneSource::kind() const { return "radio"; }

bool RetrowaveOneSource::canList() const { return true; }
bool RetrowaveOneSource::canDownload() const { return false; }

void RetrowaveOneSource::reset() {
    m_delivered = false;
    m_activeTrackId = "";
    m_poll->stop();
}

void RetrowaveOneSource::loadMore() {
    if (m_delivered) {
        emit tracksLoaded(QVariantList());
        return;
    }

    QVariantMap map;
    map["id"] = id() + ":" + kMount;
    map["title"] = name();
    map["author"] = name();
    map["name"] = name();
    map["tags"] = QStringList() << "retrowave";
    map["duration"] = 0;
    map["streamUrl"] = kStreamUrl;
    map["artworkUrl"] = "";   // no artwork in this feed
    map["bArtworkUrl"] = "";
    map["favourite"] = false;
    map["filename"] = "";

    QVariantList tracks;
    tracks.append(map);

    m_delivered = true;
    qDebug() << "===>>> RetrowaveOneSource#loadMore stations: 1" << endl;
    emit tracksLoaded(tracks);
}

void RetrowaveOneSource::setActiveTrack(const QString& trackId) {
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

void RetrowaveOneSource::onPollTimeout() {
    requestNowPlaying();
}

void RetrowaveOneSource::requestNowPlaying() {
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

void RetrowaveOneSource::onStatusReply() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    QString trackId = reply->property("trackId").toString();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "===>>> RetrowaveOneSource#onStatusReply error: " << reply->errorString() << endl;
        reply->deleteLater();
        return;
    }

    if (trackId != m_activeTrackId) {
        reply->deleteLater();
        return;
    }

    JsonDataAccess jda;
    QVariantMap stats = jda.loadFromBuffer(reply->readAll()).toMap().value("icestats").toMap();

    // A single mount comes back as an object, several as a list.
    QVariantList sources = stats.value("source").toList();
    if (sources.isEmpty() && stats.contains("source")) {
        sources.append(stats.value("source"));
    }

    foreach(QVariant var, sources) {
        QVariantMap source = var.toMap();

        // Icecast packs it all into one field as "Artist - Title".
        const QString nowPlaying = source.value("title").toString().trimmed();
        if (nowPlaying.isEmpty()) {
            continue;
        }

        QString artist;
        QString song = nowPlaying;
        const int sep = nowPlaying.indexOf(" - ");
        if (sep > 0) {
            artist = nowPlaying.left(sep).trimmed();
            song = nowPlaying.mid(sep + 3).trimmed();
        }

        QVariantMap fields;
        fields["title"] = artist.isEmpty() ? song : (artist + " " + QString(QChar(0x2013)) + " " + song);
        fields["author"] = artist;
        fields["name"] = song;

        qDebug() << "===>>> RetrowaveOneSource: on air " << fields.value("title").toString() << endl;
        emit trackUpdated(trackId, fields);
        break;
    }

    reply->deleteLater();
}
