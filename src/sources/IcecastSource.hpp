/*
 * IcecastSource - a radio service that is a plain Icecast server.
 *
 * Icecast publishes every mount and what is playing on it in one status-json.xsl,
 * so a subclass only has to say where that file is and which mounts to offer.
 * Mounts are listed by the subclass rather than taken from the status document,
 * because servers commonly expose low-bitrate and legacy duplicates of the same
 * station, which would show up as separate rows in the list.
 */

#ifndef ICECASTSOURCE_HPP_
#define ICECASTSOURCE_HPP_

#include "RadioSource.hpp"

class IcecastSource: public RadioSource {
    Q_OBJECT
public:
    IcecastSource(QObject* parent = 0);

protected:
    // Absolute URL of the server's status-json.xsl.
    virtual QString statusUrl() const = 0;

    QString nowPlayingUrl(const QString& stationKey) const;
    bool parseNowPlaying(const QByteArray& body, const QString& stationKey,
                         QVariantMap& fields) const;
};

#endif /* ICECASTSOURCE_HPP_ */
