/*
 * TrackController.hpp
 *
 *  Created on: Jun 30, 2017
 *      Author: misha
 */

#ifndef TRACKCONTROLLER_HPP_
#define TRACKCONTROLLER_HPP_

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

class TrackController: public QObject {
    Q_OBJECT
public:
    TrackController(QObject* parent = 0);
    virtual ~TrackController();

    Q_INVOKABLE void updateNowPlaying(const QString& artist, const QString& track);
    Q_INVOKABLE void scrobble(const QString& artist, const QString& track, const int& timestamp);
    Q_INVOKABLE void love(const QString& artist, const QString& track);

    Q_SIGNALS:
        void nowPlayingUpdated();
        void scrobbled();
        void loved();

private slots:
    void onNowPlayingUpdated();
    void onScrobbled();
    void onLoved();
    void onError(QNetworkReply::NetworkError e);

private:
    QNetworkAccessManager* m_pNetwork;
};

#endif /* TRACKCONTROLLER_HPP_ */
