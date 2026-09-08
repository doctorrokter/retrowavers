/*
 * RetrowaveOneSource.cpp - see RetrowaveOneSource.hpp.
 */

#include "RetrowaveOneSource.hpp"

// Baked-in address - see the caveat in the header.
static const char* kStatusUrl = "http://77.108.192.88:8000/status-json.xsl";
static const char* kStreamUrl = "http://77.108.192.88:8000/stream";
static const char* kMount = "stream";

RetrowaveOneSource::RetrowaveOneSource(QObject* parent) : IcecastSource(parent) {}

QString RetrowaveOneSource::id() const { return "retrowaveone"; }
QString RetrowaveOneSource::name() const { return "Retrowave.One"; }

QString RetrowaveOneSource::statusUrl() const { return kStatusUrl; }

void RetrowaveOneSource::requestStations() {
    Station station;
    station.key = kMount;
    station.title = name();
    station.genre = "retrowave";
    station.streamUrl = kStreamUrl;

    QList<Station> stations;
    stations.append(station);
    deliverStations(stations);
}
