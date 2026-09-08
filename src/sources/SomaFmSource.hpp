/*
 * SomaFmSource - SomaFM's radio stations as a track source.
 *
 * The second source, and the one that proves the abstraction: a station is simply
 * a "track" with no duration whose metadata changes while it plays. The list view,
 * the player and the model never learn that anything is different.
 *
 * Three things about SomaFM shape this class:
 *
 *   1. One request gets everything. channels.json carries all 46 stations with
 *      titles, genres, listener counts and 512px logos - no per-station call.
 *
 *   2. The stream URL is predictable: http://ice1.somafm.com/<id>-128-mp3. The
 *      JSON only offers .pls playlists, which would mean a request per station;
 *      the direct form was verified against several channels instead. Note it is
 *      PLAIN HTTP - mm-renderer fetches the stream itself and never sees the CA
 *      bundle installed in main(), so http sidesteps the whole TLS question.
 *
 *   3. What is on air comes from songs/<id>.xml, polled only while one of our
 *      stations is actually playing (see setActiveTrack).
 */

#ifndef SOMAFMSOURCE_HPP_
#define SOMAFMSOURCE_HPP_

#include "ITrackSource.hpp"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVariantMap>
#include <QString>

class QTimer;

class SomaFmSource: public ITrackSource {
    Q_OBJECT
public:
    SomaFmSource(QObject* parent = 0);
    virtual ~SomaFmSource();

    QString id() const;
    QString name() const;
    QString kind() const;

    bool canList() const;
    bool canDownload() const;   // false: there is no file behind a live stream

    void loadMore();
    void reset();
    void setActiveTrack(const QString& trackId);

private slots:
    void onChannelsReply();
    void onNowPlayingReply();
    void onPollTimeout();

private:
    QNetworkAccessManager* m_network;
    QTimer* m_poll;
    bool m_inFlight;
    bool m_delivered;          // channels.json is a one-shot list, not a page
    QString m_activeTrackId;   // "soma:<channel>" or "" when we are not on air

    QString channelIdOf(const QString& trackId) const;
    void requestNowPlaying();
    QVariantMap toTrackMap(const QVariantMap& channel) const;
};

#endif /* SOMAFMSOURCE_HPP_ */
