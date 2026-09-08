/*
 * NightrideSource.cpp - see NightrideSource.hpp.
 */

#include "NightrideSource.hpp"

#include <bb/data/JsonDataAccess>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>
#include <QDebug>

using namespace bb::data;

static const char* kMetaUrl = "https://nightride.fm/meta";

// Plain http: mm-renderer fetches the stream itself and never sees our CA bundle.
static const char* kStreamUrl = "http://stream.nightride.fm/%1.mp3";

// The event stream never ends by itself - if our station has not appeared by now,
// give up and try again on the next poll.
static const int kMetaTimeoutMs = 8000;

// id, display name, genre.
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

NightrideSource::NightrideSource(QObject* parent) : RadioSource(parent), m_metaReply(NULL) {}

QString NightrideSource::id() const { return "nightride"; }
QString NightrideSource::name() const { return "Nightride FM"; }

void NightrideSource::requestStations() {
    QList<Station> stations;
    for (int i = 0; i < kStationCount; ++i) {
        Station station;
        station.key = kStations[i][0];
        station.title = kStations[i][1];
        station.genre = kStations[i][2];
        station.streamUrl = QString(kStreamUrl).arg(station.key);
        stations.append(station);
    }
    deliverStations(stations);
}

void NightrideSource::closeMetaReply() {
    if (m_metaReply != NULL) {
        m_metaReply->abort();
        m_metaReply->deleteLater();
        m_metaReply = NULL;
    }
    m_metaBuffer.clear();
}

void NightrideSource::requestNowPlaying(const QString& stationKey) {
    if (stationKey.isEmpty() || m_metaReply != NULL) {
        return;   // nothing of ours on air, or a read is already running
    }

    QNetworkRequest req;
    req.setUrl(QUrl(kMetaUrl));

    m_metaBuffer.clear();
    m_metaReply = network()->get(req);
    m_metaReply->setProperty("trackId", activeTrackId());
    m_metaReply->setProperty("stationKey", stationKey);

    bool res = QObject::connect(m_metaReply, SIGNAL(readyRead()), this, SLOT(onMetaReadyRead()));
    Q_ASSERT(res);
    res = QObject::connect(m_metaReply, SIGNAL(finished()), this, SLOT(onMetaFinished()));
    Q_ASSERT(res);
    Q_UNUSED(res);

    QTimer::singleShot(kMetaTimeoutMs, m_metaReply, SLOT(abort()));
}

void NightrideSource::onMetaReadyRead() {
    if (m_metaReply == NULL) {
        return;
    }

    m_metaBuffer.append(m_metaReply->readAll());
    const QString station = m_metaReply->property("stationKey").toString();
    const QString trackId = m_metaReply->property("trackId").toString();

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

            const QString song = entry.value("title").toString();
            if (song.isEmpty()) {
                continue;
            }

            const QString artist = entry.value("artist").toString();
            QVariantMap fields;
            fields["title"] = joinArtistTitle(artist, song);
            fields["author"] = artist;
            fields["name"] = song;

            publishNowPlaying(trackId, fields);

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
