/*
 * FacebookController.cpp
 *
 *  Created on: Aug 15, 2017
 *      Author: misha
 */

#include "FacebookController.hpp"
#include "../models/Track.hpp"
#include "../config/AppConfig.hpp"
#include <QUrl>
#include <QNetworkRequest>

#define FB_API_ENDPOINT "https://graph.facebook.com/v2.10"

FacebookController::FacebookController(QObject* parent) : QObject(parent) {
    m_pNetwork = new QNetworkAccessManager(this);
    m_pToast = new SystemToast(this);
}

FacebookController::~FacebookController() {
    m_pNetwork->deleteLater();
    m_pToast->deleteLater();
}

void FacebookController::share(const QVariantMap& track) const {
    Track t;
    t.fromMap(track);

    QUrl url(QString(FB_API_ENDPOINT).append("/me/feed"));
    QUrl params;
    params.addQueryItem("access_token", AppConfig::getStatic("fb_access_token").toString());
    params.addQueryItem("link", t.getArtworkUrl());
    params.addQueryItem("message", QString(t.getTitle() + "\n\n").append(tr("Now listening in Retrowavers: The Legacy app on my BlackBerry 10 smartphone")));

    QNetworkRequest req;
    req.setUrl(url);
    req.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    qDebug() << "===>>> FB " << url << endl;
    qDebug() << "===>>> FB " << params.encodedQuery() << endl;

    QNetworkReply* reply = m_pNetwork->post(req, params.encodedQuery());
    bool res = QObject::connect(reply, SIGNAL(finished()), this, SLOT(onShare()));
    Q_ASSERT(res);
    res = QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

void FacebookController::onShare() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());

    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << reply->readAll() << endl;
        m_pToast->setBody(tr("FB status updated"));
        m_pToast->show();
        emit shared();
    }

    reply->deleteLater();
}

void FacebookController::onError(QNetworkReply::NetworkError e) {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    qDebug() << "===>>> FacebookController#onError: " << e << endl;
    qDebug() << "===>>> FacebookController#onError: " << reply->errorString() << endl;
    reply->deleteLater();
}

