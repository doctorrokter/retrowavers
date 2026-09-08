/*
 * WaveRadioSource.cpp - see WaveRadioSource.hpp.
 */

#include "WaveRadioSource.hpp"

static const char* kStatusUrl = "http://station.waveradio.org/status-json.xsl";
static const char* kStreamUrl = "http://station.waveradio.org/%1";

// mount, display name, genre. "witch" is AAC - the only mount here without an mp3
// twin - which mm-renderer handles as happily as it does mp3.
static const char* kStations[][3] = {
    { "soviet.mp3",    "Sovietwave",  "sovietwave" },
    { "provodach.mp3", "Provoda.ch",  "witch house" },
    { "witch",         "Witch House", "witch house" }
};
static const int kStationCount = sizeof(kStations) / sizeof(kStations[0]);

WaveRadioSource::WaveRadioSource(QObject* parent) : IcecastSource(parent) {}

QString WaveRadioSource::id() const { return "waveradio"; }
QString WaveRadioSource::name() const { return "WaveRadio"; }

QString WaveRadioSource::statusUrl() const { return kStatusUrl; }

void WaveRadioSource::requestStations() {
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
