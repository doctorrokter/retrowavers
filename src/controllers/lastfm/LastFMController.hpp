/*
 * LastFMController.hpp
 *
 *  Created on: Jun 29, 2017
 *      Author: misha
 */

#ifndef LASTFMCONTROLLER_HPP_
#define LASTFMCONTROLLER_HPP_

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include "TrackController.hpp"

class LastFMController: public QObject {
    Q_OBJECT
    Q_PROPERTY(TrackController* track READ getTrackController)
public:
    LastFMController(QObject* parent = 0);
    virtual ~LastFMController();

    Q_INVOKABLE void authenticate(const QString& username, const QString& password);

    Q_INVOKABLE TrackController* getTrackController() const;

    Q_SIGNALS:
        void authenticationFinished(const QString& message, const bool& success);

private slots:
    void onAuthenticate();
    void onError(QNetworkReply::NetworkError e);

private:
    QNetworkAccessManager* m_pNetwork;

    TrackController* m_pTrack;
};

#endif /* LASTFMCONTROLLER_HPP_ */
