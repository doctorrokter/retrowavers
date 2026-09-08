/*
 * NightrideSource - Nightride FM: eleven stations, all of them squarely in this
 * app's genre (synthwave, darksynth, chillsynth, spacesynth, horrorsynth...).
 *
 * Two conveniences and one quirk:
 *
 *   - The streams are PLAIN HTTP (http://stream.nightride.fm/<id>.mp3), so
 *     mm-renderer fetches them without touching TLS at all.
 *
 *   - There is no station-list endpoint, so the list lives in this file. That is
 *     fine: it changes about never, and hardcoding it means a proper display name
 *     and genre per station instead of whatever an API would hand us.
 *
 *   - "What is on air" comes from /meta, which is a server-sent EVENT STREAM, not
 *     a request that ends. It opens with a snapshot of every station, so we read
 *     until the station we care about shows up and then abort the request - see
 *     onMetaReadyRead().
 */

#ifndef NIGHTRIDESOURCE_HPP_
#define NIGHTRIDESOURCE_HPP_

#include "ITrackSource.hpp"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QByteArray>
#include <QVariantMap>

class QTimer;

class NightrideSource: public ITrackSource {
    Q_OBJECT
public:
    NightrideSource(QObject* parent = 0);
    virtual ~NightrideSource();

    QString id() const;
    QString name() const;
    QString kind() const;

    bool canList() const;
    bool canDownload() const;

    void loadMore();
    void reset();
    void setActiveTrack(const QString& trackId);

private slots:
    void onMetaReadyRead();
    void onMetaFinished();
    void onPollTimeout();

private:
    QNetworkAccessManager* m_network;
    QTimer* m_poll;
    bool m_delivered;
    QString m_activeTrackId;
    QByteArray m_metaBuffer;
    QNetworkReply* m_metaReply;

    QString stationOf(const QString& trackId) const;
    void requestNowPlaying();
    void closeMetaReply();
};

#endif /* NIGHTRIDESOURCE_HPP_ */
