/*
 * NightrideSource - Nightride FM: eleven stations, all of them squarely in this
 * app's genre (synthwave, darksynth, chillsynth, spacesynth, horrorsynth...).
 *
 * The station list is hardcoded because there is no endpoint for it - only the
 * metadata feed. That is no loss: the list changes about never, and hardcoding it
 * gives a proper display name and genre per station.
 *
 * The one thing this service does differently from every other radio here: /meta is
 * a server-sent EVENT STREAM, not a request that ends. It opens with a snapshot of
 * every station, so requestNowPlaying() is overridden to read until our station
 * shows up and then abort the request.
 */

#ifndef NIGHTRIDESOURCE_HPP_
#define NIGHTRIDESOURCE_HPP_

#include "RadioSource.hpp"

#include <QByteArray>

class QNetworkReply;

class NightrideSource: public RadioSource {
    Q_OBJECT
public:
    NightrideSource(QObject* parent = 0);

    QString id() const;
    QString name() const;

protected:
    void requestStations();
    void requestNowPlaying(const QString& stationKey);

private slots:
    void onMetaReadyRead();
    void onMetaFinished();

private:
    QByteArray m_metaBuffer;
    QNetworkReply* m_metaReply;

    void closeMetaReply();
};

#endif /* NIGHTRIDESOURCE_HPP_ */
