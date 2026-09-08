/*
 * SynthwaveRadioSource.cpp - see SynthwaveRadioSource.hpp.
 */

#include "SynthwaveRadioSource.hpp"

#include <bb/data/JsonDataAccess>

using namespace bb::data;

static const char* kStatusUrl = "https://stream.synthwaveradio.eu/api/nowplaying/synthwaveradio.eu";
static const char* kStreamUrl = "https://stream.synthwaveradio.eu/listen/synthwaveradio.eu/radio.mp3";
static const char* kStationKey = "main";

SynthwaveRadioSource::SynthwaveRadioSource(QObject* parent) : RadioSource(parent) {}

QString SynthwaveRadioSource::id() const { return "synthwaveradio"; }
QString SynthwaveRadioSource::name() const { return "SynthwaveRadio.eu"; }

void SynthwaveRadioSource::requestStations() {
    Station station;
    station.key = kStationKey;
    station.title = name();
    station.genre = "synthwave";
    station.streamUrl = kStreamUrl;
    // artworkUrl stays empty: per-track art arrives with the first poll.

    QList<Station> stations;
    stations.append(station);
    deliverStations(stations);
}

QString SynthwaveRadioSource::nowPlayingUrl(const QString& stationKey) const {
    Q_UNUSED(stationKey);
    return kStatusUrl;
}

bool SynthwaveRadioSource::parseNowPlaying(const QByteArray& body, const QString& stationKey,
                                           QVariantMap& fields) const {
    Q_UNUSED(stationKey);

    JsonDataAccess jda;
    QVariantMap root = jda.loadFromBuffer(body).toMap();
    QVariantMap song = root.value("now_playing").toMap().value("song").toMap();

    const QString artist = song.value("artist").toString();
    const QString title = song.value("title").toString();
    if (title.isEmpty()) {
        return false;
    }

    fields["title"] = joinArtistTitle(artist, title);
    fields["author"] = artist;
    fields["name"] = title;

    const QString art = song.value("art").toString();
    if (!art.isEmpty()) {
        fields["artworkUrl"] = art;
    }

    return true;
}
