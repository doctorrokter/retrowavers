/*
 * TrackController.cpp
 *
 *  Created on: Jun 30, 2017
 *      Author: misha
 */

#define API_NAMESPACE "track"

#include "TrackController.hpp"
#include "LastFMCommon.hpp"
#include "../../config/AppConfig.hpp"
#include <QUrl>
#include <QCryptographicHash>
#include <QDebug>

TrackController::TrackController(QObject* parent) : QObject(parent) {
    m_pNetwork = new QNetworkAccessManager(this);
}

TrackController::~TrackController() {
    m_pNetwork->deleteLater();
}

void TrackController::updateNowPlaying(const QString& artist, const QString& track) {
    QNetworkRequest req;

    QUrl url(API_ROOT);
    req.setUrl(url);
    req.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    QUrl body;
    QString sk = AppConfig::getStatic(LAST_FM_KEY).toString();
    QString sig = QString("api_key").append(API_KEY)
                .append("artist").append(artist.toUtf8())
                .append("method").append(TRACK_UPDATE_NOW_PLAYING)
                .append("sk").append(sk)
                .append("track").append(track.toUtf8())
                .append(SECRET);
    QString hash = QCryptographicHash::hash(sig.toAscii(), QCryptographicHash::Md5).toHex();

    body.addQueryItem("method", TRACK_UPDATE_NOW_PLAYING);
    body.addQueryItem("artist", artist.toUtf8());
    body.addQueryItem("track", track.toUtf8());
    body.addQueryItem("api_key", API_KEY);
    body.addQueryItem("sk", sk);
    body.addQueryItem("api_sig", hash);

    QNetworkReply* reply = m_pNetwork->post(req, body.encodedQuery());
    bool res = QObject::connect(reply, SIGNAL(finished()), this, SLOT(onNowPlayingUpdated()));
    Q_ASSERT(res);
    res = QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

void TrackController::onNowPlayingUpdated() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());

    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << reply->readAll() << endl;
    }

    reply->deleteLater();
}

void TrackController::onError(QNetworkReply::NetworkError e) {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    qDebug() << "===>>> TrackController#onError " << e << endl;
    qDebug() << "===>>> TrackController#onError " << reply->errorString() << endl;
    reply->deleteLater();
}
