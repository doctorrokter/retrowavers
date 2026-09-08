/*
 * WaveRadioSource - waveradio.org, a small Icecast server whose Sovietwave station
 * is exactly the corner of the genre nothing else here covers.
 *
 * Everything mechanical lives in IcecastSource / RadioSource; this is just the
 * server address and which mounts to offer.
 */

#ifndef WAVERADIOSOURCE_HPP_
#define WAVERADIOSOURCE_HPP_

#include "IcecastSource.hpp"

class WaveRadioSource: public IcecastSource {
    Q_OBJECT
public:
    WaveRadioSource(QObject* parent = 0);

    QString id() const;
    QString name() const;

protected:
    void requestStations();
    QString statusUrl() const;
};

#endif /* WAVERADIOSOURCE_HPP_ */
