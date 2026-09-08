/*
 * SomaFmSource.cpp - see SomaFmSource.hpp.
 */

#include "SomaFmSource.hpp"

#include <bb/data/JsonDataAccess>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QUrl>
#include <QDebug>

using namespace bb::data;

static const char* kChannelsUrl = "https://somafm.com/channels.json";
static const char* kNowPlayingUrl = "https://somafm.com/songs/%1.xml";

// Plain http on purpose: mm-renderer fetches the stream itself and never sees the
// CA bundle installed in main().
static const char* kStreamUrl = "http://ice1.somafm.com/%1-128-mp3";

SomaFmSource::SomaFmSource(QObject* parent) : RadioSource(parent), m_inFlight(false) {}

QString SomaFmSource::id() const { return "soma"; }
QString SomaFmSource::name() const { return "SomaFM"; }

void SomaFmSource::requestStations() {
    if (m_inFlight) {
        return;
    }
    m_inFlight = true;

    QNetworkRequest req;
    req.setUrl(QUrl(kChannelsUrl));

    QNetworkReply* reply = network()->get(req);
    bool res = QObject::connect(reply, SIGNAL(finished()), this, SLOT(onChannelsReply()));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

void SomaFmSource::onChannelsReply() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    m_inFlight = false;

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "===>>> SomaFmSource channels error: " << reply->errorString() << endl;
        emit loadFailed(reply->errorString());
        reply->deleteLater();
        return;
    }

    JsonDataAccess jda;
    QVariantList channels = jda.loadFromBuffer(reply->readAll()).toMap().value("channels").toList();

    QList<Station> stations;
    foreach(QVariant var, channels) {
        QVariantMap channel = var.toMap();

        Station station;
        station.key = channel.value("id").toString();
        station.title = channel.value("title").toString();
        // "electronic|specials" is SomaFM's own genre field; the first one will do.
        station.genre = channel.value("genre").toString().section("|", 0, 0);
        station.streamUrl = QString(kStreamUrl).arg(station.key);
        station.artworkUrl = channel.value("xlimage").toString();

        stations.append(station);
    }

    deliverStations(stations);
    reply->deleteLater();
}

QString SomaFmSource::nowPlayingUrl(const QString& stationKey) const {
    return QString(kNowPlayingUrl).arg(stationKey);
}

bool SomaFmSource::parseNowPlaying(const QByteArray& body, const QString& stationKey,
                                   QVariantMap& fields) const {
    Q_UNUSED(stationKey);

    // The feed lists the last ~20 songs, newest first; the first one is on air.
    QString artist;
    QString song;
    QString albumArt;

    QXmlStreamReader xml(body);
    bool inSong = false;
    while (!xml.atEnd() && song.isEmpty()) {
        xml.readNext();
        if (!xml.isStartElement()) {
            continue;
        }

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

    if (song.isEmpty()) {
        return false;
    }

    fields["title"] = joinArtistTitle(artist, song);
    fields["author"] = artist;
    fields["name"] = song;
    if (!albumArt.isEmpty()) {
        fields["artworkUrl"] = albumArt;
    }

    return true;
}
