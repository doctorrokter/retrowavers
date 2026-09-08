/*
 * NightrideSource.cpp - see NightrideSource.hpp.
 */

#include "NightrideSource.hpp"

#include <bb/data/JsonDataAccess>

#include <QNetworkRequest>
#include <QUrl>
#include <QTimer>
#include <QStringList>
#include <QDebug>

using namespace bb::data;

// The event stream with what is playing on every station.
static const char* kMetaUrl = "https://nightride.fm/meta";

// Plain http: mm-renderer fetches the stream itself and never sees our CA bundle.
static const char* kStreamUrl = "http://stream.nightride.fm/%1.mp3";

static const int kPollMs = 30000;
static const int kMetaTimeoutMs = 8000;

// id, display name, genre. Nightride has no station-list endpoint; see the header.
static const char* kStations[][3] = {
    { "nightride",   "Nightride FM",  "synthwave" },
    { "chillsynth",  "Chillsynth",    "chillwave" },
    { "darksynth",   "Darksynth",     "darksynth" },
    { "horrorsynth", "Horrorsynth",   "horror" },
    { "spacesynth",  "Spacesynth",    "spacesynth" },
    { "datawave",    "Datawave",      "cyber" },
    { "d-notive",    "D-Notive",      "dreamwave" },
    { "ebsm",        "EBSM",          "ebm" },
    { "rekt",        "Rekt",          "heavy" },
    { "rektify",     "Rektify",       "heavy" },
    { "rektory",     "Rektory",       "experimental" }
};
static const int kStationCount = sizeof(kStations) / sizeof(kStations[0]);

NightrideSource::NightrideSource(QObject* parent) : ITrackSource(parent),
        m_network(new QNetworkAccessManager(this)), m_poll(new QTimer(this)),
        m_delivered(false), m_metaReply(NULL) {
    m_poll->setInterval(kPollMs);
    bool res = QObject::connect(m_poll, SIGNAL(timeout()), this, SLOT(onPollTimeout()));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

NightrideSource::~NightrideSource() {
    m_poll->stop();
    closeMetaReply();
    m_network->deleteLater();
}

QString NightrideSource::id() const { return "nightride"; }
QString NightrideSource::name() const { return "Nightride FM"; }
QString NightrideSource::kind() const { return "radio"; }

bool NightrideSource::canList() const { return true; }
bool NightrideSource::canDownload() const { return false; }

void NightrideSource::reset() {
    m_delivered = false;
    m_activeTrackId = "";
    m_poll->stop();
    closeMetaReply();
}

void NightrideSource::loadMore() {
    if (m_delivered) {
        emit tracksLoaded(QVariantList());   // one fixed list, no paging
        return;
    }

    QVariantList tracks;
    for (int i = 0; i < kStationCount; ++i) {
        QString station = kStations[i][0];

        QVariantMap map;
        map["id"] = id() + ":" + station;
        map["title"] = QString(kStations[i][1]);
        map["author"] = name();
        map["name"] = QString(kStations[i][1]);
        map["tags"] = QStringList() << kStations[i][2];
        map["duration"] = 0;                      // live: no length
        map["streamUrl"] = QString(kStreamUrl).arg(station);
        map["artworkUrl"] = "";                   // no station art; the default cassette shows
        map["bArtworkUrl"] = "";
        map["favourite"] = false;
        map["filename"] = "";

        tracks.append(map);
    }

    m_delivered = true;
    qDebug() << "===>>> NightrideSource#loadMore stations: " << tracks.size() << endl;
    emit tracksLoaded(tracks);
}

QString NightrideSource::stationOf(const QString& trackId) const {
    const QString prefix = id() + ":";
    return trackId.startsWith(prefix) ? trackId.mid(prefix.length()) : QString("");
}

void NightrideSource::setActiveTrack(const QString& trackId) {
    if (stationOf(trackId).isEmpty()) {
        m_activeTrackId = "";
        m_poll->stop();
        closeMetaReply();
        return;
    }

    if (m_activeTrackId == trackId) {
        return;
    }

    m_activeTrackId = trackId;
    requestNowPlaying();
    m_poll->start();
}

void NightrideSource::onPollTimeout() {
    requestNowPlaying();
}

void NightrideSource::closeMetaReply() {
    if (m_metaReply != NULL) {
        m_metaReply->abort();
        m_metaReply->deleteLater();
        m_metaReply = NULL;
    }
    m_metaBuffer.clear();
}

void NightrideSource::requestNowPlaying() {
    if (stationOf(m_activeTrackId).isEmpty() || m_metaReply != NULL) {
        return;   // nothing on air of ours, or a read is already running
    }

    QNetworkRequest req;
    req.setUrl(QUrl(kMetaUrl));

    m_metaBuffer.clear();
    m_metaReply = m_network->get(req);
    bool res = QObject::connect(m_metaReply, SIGNAL(readyRead()), this, SLOT(onMetaReadyRead()));
    Q_ASSERT(res);
    res = QObject::connect(m_metaReply, SIGNAL(finished()), this, SLOT(onMetaFinished()));
    Q_ASSERT(res);
    Q_UNUSED(res);

    // The stream never ends by itself - if our station has not appeared by now,
    // give up and try again on the next poll.
    QTimer::singleShot(kMetaTimeoutMs, m_metaReply, SLOT(abort()));
}

void NightrideSource::onMetaReadyRead() {
    if (m_metaReply == NULL) {
        return;
    }

    m_metaBuffer.append(m_metaReply->readAll());
    const QString station = stationOf(m_activeTrackId);

    // Event-stream framing: "data: <json>\n". Take whole lines only; the last,
    // possibly partial one stays in the buffer for the next chunk.
    while (true) {
        const int eol = m_metaBuffer.indexOf('\n');
        if (eol < 0) {
            break;
        }

        const QByteArray line = m_metaBuffer.left(eol).trimmed();
        m_metaBuffer.remove(0, eol + 1);
        if (!line.startsWith("data:")) {
            continue;
        }

        JsonDataAccess jda;
        QVariantList entries = jda.loadFromBuffer(line.mid(5).trimmed()).toList();
        foreach(QVariant var, entries) {
            QVariantMap entry = var.toMap();
            if (entry.value("station").toString().compare(station) != 0) {
                continue;
            }

            const QString artist = entry.value("artist").toString();
            const QString song = entry.value("title").toString();
            if (song.isEmpty()) {
                continue;
            }

            QVariantMap fields;
            fields["title"] = artist.isEmpty() ? song : (artist + " " + QString(QChar(0x2013)) + " " + song);
            fields["author"] = artist;
            fields["name"] = song;

            qDebug() << "===>>> NightrideSource: on air " << fields.value("title").toString() << endl;
            emit trackUpdated(m_activeTrackId, fields);

            closeMetaReply();   // we have what we came for; the stream would run forever
            return;
        }
    }
}

void NightrideSource::onMetaFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    if (reply == NULL) {
        return;
    }

    if (reply == m_metaReply) {
        m_metaReply = NULL;
        m_metaBuffer.clear();
    }
    reply->deleteLater();
}
