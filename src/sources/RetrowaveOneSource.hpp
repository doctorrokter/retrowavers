/*
 * RetrowaveOneSource - retrowave.one, one station on a plain Icecast server.
 *
 * CAVEAT: the site's player reaches the stream by IP, and no hostname resolves to
 * it (stream./radio.retrowave.one do not exist). The address is therefore baked
 * into the .cpp - if the host ever moves, this station stops working and the
 * constant has to be updated. It is the only such assumption in the app.
 */

#ifndef RETROWAVEONESOURCE_HPP_
#define RETROWAVEONESOURCE_HPP_

#include "IcecastSource.hpp"

class RetrowaveOneSource: public IcecastSource {
    Q_OBJECT
public:
    RetrowaveOneSource(QObject* parent = 0);

    QString id() const;
    QString name() const;

protected:
    void requestStations();
    QString statusUrl() const;
};

#endif /* RETROWAVEONESOURCE_HPP_ */
