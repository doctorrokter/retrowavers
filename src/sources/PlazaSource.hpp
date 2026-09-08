/*
 * PlazaSource - Nightwave Plaza, a single vaporwave station.
 *
 * One station is a thin excuse for a source, but it earns its place: unlike most of
 * the radio services, Plaza's API reports the ARTWORK of the current track, so this
 * is one of the two radios where the cassette and the blurred backdrop come alive
 * instead of falling back to the default cover.
 *
 * (api.plaza.one/status also carries `length`, `position` and `listeners` - enough
 * for a real progress bar, if that is ever wanted on radio.)
 */

#ifndef PLAZASOURCE_HPP_
#define PLAZASOURCE_HPP_

#include "RadioSource.hpp"

class PlazaSource: public RadioSource {
    Q_OBJECT
public:
    PlazaSource(QObject* parent = 0);

    QString id() const;
    QString name() const;

protected:
    void requestStations();
    QString nowPlayingUrl(const QString& stationKey) const;
    bool parseNowPlaying(const QByteArray& body, const QString& stationKey,
                         QVariantMap& fields) const;
};

#endif /* PLAZASOURCE_HPP_ */
