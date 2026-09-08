/*
 * WaveRadioSource.cpp - see WaveRadioSource.hpp.
 */

#include "WaveRadioSource.hpp"

#include <bb/data/JsonDataAccess>

#include <QNetworkRequest>
#include <QUrl>
#include <QTimer>
#include <QStringList>
#include <QDebug>

using namespace bb::data;

static const char* kStatusUrl = "http://station.waveradio.org/status-json.xsl";
static const char* kStreamUrl = "http://station.waveradio.org/%1";

static const int kPollMs = 30000;

// mount, display name, genre. "witch" is AAC - the only mount here without an mp3
// twin - which mm-renderer handles as happily as it does mp3.
static const char* kStations[][3] = {
    { "soviet.mp3",    "Sovietwave",  "sovietwave" },
    { "provodach.mp3", "Provoda.ch",  "witch house" },
    { "witch",         "Witch House", "witch house" }
};
static const int kStationCount = sizeof(kStations) / sizeof(kStations[0]);

WaveRadioSource::WaveRadioSource(QObject* parent) : ITrackSource(parent),
        m_network(new QNetworkAccessManager(this)), m_poll(new QTimer(this)), m_delivered(false) {
    m_poll->setInterval(kPollMs);
    bool res = QObject::connect(m_poll, SIGNAL(timeout()), this, SLOT(onPollTimeout()));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

WaveRadioSource::~WaveRadioSource() {
    m_poll->stop();
    m_network->deleteLater();
}

QString WaveRadioSource::id() const { return "waveradio"; }
QString WaveRadioSource::name() const { return "WaveRadio"; }
QString WaveRadioSource::kind() const { return "radio"; }

bool WaveRadioSource::canList() const { return true; }
bool WaveRadioSource::canDownload() const { return false; }

void WaveRadioSource::reset() {
    m_delivered = false;
    m_activeTrackId = "";
    m_poll->stop();
}

void WaveRadioSource::loadMore() {
    if (m_delivered) {
        emit tracksLoaded(QVariantList());
        return;
    }

    QVariantList tracks;
    for (int i = 0; i < kStationCount; ++i) {
        QString mount = kStations[i][0];

        QVariantMap map;
        map["id"] = id() + ":" + mount;
        map["title"] = QString(kStations[i][1]);
        map["author"] = name();
        map["name"] = QString(kStations[i][1]);
        map["tags"] = QStringList() << kStations[i][2];
        map["duration"] = 0;
        map["streamUrl"] = QString(kStreamUrl).arg(mount);
        map["artworkUrl"] = "";
        map["bArtworkUrl"] = "";
        map["favourite"] = false;
        map["filename"] = "";

        tracks.append(map);
    }

    m_delivered = true;
    qDebug() << "===>>> WaveRadioSource#loadMore stations: " << tracks.size() << endl;
    emit tracksLoaded(tracks);
}

QString WaveRadioSource::mountOf(const QString& trackId) const {
    const QString prefix = id() + ":";
    return trackId.startsWith(prefix) ? trackId.mid(prefix.length()) : QString("");
}

void WaveRadioSource::setActiveTrack(const QString& trackId) {
    if (mountOf(trackId).isEmpty()) {
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

void WaveRadioSource::onPollTimeout() {
    requestNowPlaying();
}

void WaveRadioSource::requestNowPlaying() {
    if (mountOf(m_activeTrackId).isEmpty()) {
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

void WaveRadioSource::onStatusReply() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    QString trackId = reply->property("trackId").toString();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "===>>> WaveRadioSource#onStatusReply error: " << reply->errorString() << endl;
        reply->deleteLater();
        return;
    }

    if (trackId != m_activeTrackId) {
        reply->deleteLater();   // station switched while the request was in flight
        return;
    }

    JsonDataAccess jda;
    QVariantMap stats = jda.loadFromBuffer(reply->readAll()).toMap().value("icestats").toMap();

    // One mount is reported as an object, several as a list.
    QVariantList sources = stats.value("source").toList();
    if (sources.isEmpty() && stats.contains("source")) {
        sources.append(stats.value("source"));
    }

    const QString mount = mountOf(trackId);
    foreach(QVariant var, sources) {
        QVariantMap source = var.toMap();
        if (!source.value("listenurl").toString().endsWith("/" + mount)) {
            continue;
        }

        // Icecast puts the whole thing in one field as "Artist - Title".
        const QString nowPlaying = source.value("title").toString().trimmed();
        if (nowPlaying.isEmpty()) {
            break;
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

        qDebug() << "===>>> WaveRadioSource: on air " << fields.value("title").toString() << endl;
        emit trackUpdated(trackId, fields);
        break;
    }

    reply->deleteLater();
}
