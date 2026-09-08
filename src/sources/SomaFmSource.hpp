/*
 * SomaFmSource - SomaFM's 46 stations.
 *
 * The only radio service here whose station list is fetched rather than hardcoded:
 * channels.json returns every station in one request, with titles, genres and
 * 512px logos - so this is also the only radio with station artwork (per-station,
 * not per-track: SomaFM's song feed almost never fills in album art).
 *
 * The JSON offers .pls playlists, which would mean a request per station; the
 * direct ice1…-128-mp3 form is used instead, verified against several channels.
 */

#ifndef SOMAFMSOURCE_HPP_
#define SOMAFMSOURCE_HPP_

#include "RadioSource.hpp"

class SomaFmSource: public RadioSource {
    Q_OBJECT
public:
    SomaFmSource(QObject* parent = 0);

    QString id() const;
    QString name() const;

protected:
    void requestStations();
    QString nowPlayingUrl(const QString& stationKey) const;
    bool parseNowPlaying(const QByteArray& body, const QString& stationKey,
                         QVariantMap& fields) const;

private slots:
    void onChannelsReply();

private:
    bool m_inFlight;
};

#endif /* SOMAFMSOURCE_HPP_ */
