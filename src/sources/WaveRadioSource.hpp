/*
 * WaveRadioSource - waveradio.org, a small Icecast server whose Sovietwave
 * station is exactly the corner of the genre the other services do not cover.
 *
 * Plain Icecast, which makes it the simplest source here: the mounts are fixed
 * http URLs, and status-json.xsl reports what is playing on EVERY mount in one
 * request - no per-station polling and no event stream to unpick.
 *
 * Mount names are hardcoded rather than taken from status-json.xsl, because that
 * listing also carries duplicates of each station (a low-bitrate and a legacy
 * mount of the same audio) which would show up as separate entries in the list.
 */

#ifndef WAVERADIOSOURCE_HPP_
#define WAVERADIOSOURCE_HPP_

#include "ITrackSource.hpp"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVariantMap>

class QTimer;

class WaveRadioSource: public ITrackSource {
    Q_OBJECT
public:
    WaveRadioSource(QObject* parent = 0);
    virtual ~WaveRadioSource();

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

    QString mountOf(const QString& trackId) const;
    void requestNowPlaying();
};

#endif /* WAVERADIOSOURCE_HPP_ */
