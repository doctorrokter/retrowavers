/*
 * PlazaSource - Nightwave Plaza, a single vaporwave station.
 *
 * One station is a thin excuse for a source, but it earns its place: unlike the
 * other radio services, Plaza's API reports the ARTWORK of the current track. So
 * this is the one radio where the cassette and the blurred backdrop come alive
 * instead of falling back to the default cover.
 *
 * api.plaza.one/status answers with the song (artist, title, artwork_src) in one
 * small JSON, and the stream is plain http like the rest.
 */

#ifndef PLAZASOURCE_HPP_
#define PLAZASOURCE_HPP_

#include "ITrackSource.hpp"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVariantMap>

class QTimer;

class PlazaSource: public ITrackSource {
    Q_OBJECT
public:
    PlazaSource(QObject* parent = 0);
    virtual ~PlazaSource();

    QString id() const;
    QString name() const;
    QString kind() const;

    bool canList() const;
    bool canDownload() const;

    void loadMore();
    void reset();
    void setActiveTrack(const QString& trackId);

private slots:
    void onStatusReply();
    void onPollTimeout();

private:
    QNetworkAccessManager* m_network;
    QTimer* m_poll;
    bool m_delivered;
    QString m_activeTrackId;

    void requestNowPlaying();
};

#endif /* PLAZASOURCE_HPP_ */
