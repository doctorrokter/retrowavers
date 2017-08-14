/*
 * VKController.cpp
 *
 *  Created on: Aug 14, 2017
 *      Author: misha
 */

#include <QDebug>
#include <QUrl>
#include <QNetworkRequest>
#include "VKController.hpp"
#include "../config/AppConfig.hpp"
#include "../models/Track.hpp"

#define VK_API_ENDPOINT "https://api.vk.com/method"

VKController::VKController(QObject* parent) : QObject(parent) {
    m_pNetwork = new QNetworkAccessManager(this);
    m_pToast = new SystemToast(this);
}

VKController::~VKController() {
    m_pNetwork->deleteLater();
    m_pToast->deleteLater();
}

void VKController::share(const QVariantMap& track) const {
    Track t;
    t.fromMap(track);

    QUrl url(QString(VK_API_ENDPOINT).append("/wall.post"));
    QUrl params;
    params.addQueryItem("owner_id", AppConfig::getStatic("vk_user_id").toString());
    params.addQueryItem("access_token", AppConfig::getStatic("vk_access_token").toString());
    params.addQueryItem("attachments", t.getArtworkUrl());
    params.addQueryItem("v", AppConfig::getStatic("vk_api_version").toString());
    params.addQueryItem("message", QString(t.getTitle() + "\n\n").append(tr("Now listening in Retrowavers: The Legacy app on my BlackBerry 10 smartphone")));

    QNetworkRequest req;
    req.setUrl(url);
    req.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    qDebug() << "===>>> VK " << url << endl;
    qDebug() << "===>>> VK " << params.encodedQuery() << endl;

    QNetworkReply* reply = m_pNetwork->post(req, params.encodedQuery());
    bool res = QObject::connect(reply, SIGNAL(finished()), this, SLOT(onShare()));
    Q_ASSERT(res);
    res = QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

void VKController::onShare() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());

    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << reply->readAll() << endl;
        m_pToast->setBody(tr("Record created on the wall"));
        m_pToast->show();
    }

    reply->deleteLater();
}

void VKController::onError(QNetworkReply::NetworkError e) {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    qDebug() << "===>>> VKController#onError: " << e << endl;
    qDebug() << "===>>> VKController#onError: " << reply->errorString() << endl;
    reply->deleteLater();
}

