/*
 * PlazaSource.cpp - see PlazaSource.hpp.
 */

#include "PlazaSource.hpp"

#include <bb/data/JsonDataAccess>

using namespace bb::data;

static const char* kStatusUrl = "https://api.plaza.one/status";

// Plain http, and the mount is /mp3 - /mp2 (quoted in some write-ups) is a 404.
static const char* kStreamUrl = "http://radio.plaza.one/mp3";
static const char* kStationKey = "plaza";

PlazaSource::PlazaSource(QObject* parent) : RadioSource(parent) {}

QString PlazaSource::id() const { return "plaza"; }
QString PlazaSource::name() const { return "Nightwave Plaza"; }

void PlazaSource::requestStations() {
    Station station;
    station.key = kStationKey;
    station.title = name();
    station.genre = "vaporwave";
    station.streamUrl = kStreamUrl;
    // artworkUrl stays empty: the art arrives per track with the first poll.

    QList<Station> stations;
    stations.append(station);
    deliverStations(stations);
}

QString PlazaSource::nowPlayingUrl(const QString& stationKey) const {
    Q_UNUSED(stationKey);
    return kStatusUrl;
}

bool PlazaSource::parseNowPlaying(const QByteArray& body, const QString& stationKey,
                                  QVariantMap& fields) const {
    Q_UNUSED(stationKey);

    JsonDataAccess jda;
    QVariantMap song = jda.loadFromBuffer(body).toMap().value("song").toMap();

    const QString artist = song.value("artist").toString();
    const QString title = song.value("title").toString();
    if (title.isEmpty()) {
        return false;
    }

    fields["title"] = joinArtistTitle(artist, title);
    fields["author"] = artist;
    fields["name"] = title;

    // One of only two radio services that gives us real cover art per track.
    const QString artwork = song.value("artwork_src").toString();
    if (!artwork.isEmpty()) {
        fields["artworkUrl"] = artwork;
    }

    return true;
}
