/*
 * LastFMController.cpp
 *
 *  Created on: Jun 29, 2017
 *      Author: misha
 */

#include "LastFMController.hpp"
#include <QCryptographicHash>
#include <QDebug>
#include <bb/data/JsonDataAccess>
#include <QVariantMap>
#include "LastFMCommon.hpp"

using namespace bb::data;

LastFMController::LastFMController(AppConfig* appConfig, QObject* parent) : QObject(parent) {
    m_pNetwork = new QNetworkAccessManager(this);
    m_pTrack = new TrackController(this);
    m_pAppConfig = appConfig;
}

LastFMController::~LastFMController() {
    m_pNetwork->deleteLater();
    m_pTrack->deleteLater();
}

void LastFMController::authenticate(const QString& username, const QString& password) {
    QNetworkRequest req;

    QUrl url(AUTH_ROOT);

    QByteArray body;
    url.addQueryItem("method", AUTH_METHOD);
    url.addQueryItem("username", username.toUtf8());
    url.addQueryItem("password", password.toUtf8());
    url.addQueryItem("api_key", API_KEY);
    url.addQueryItem("format", "json");

    QString sig = QString("api_key").append(API_KEY).append("method").append(AUTH_METHOD).append("password").append(password).append("username").append(username).append(SECRET);
    QString hash = QCryptographicHash::hash(sig.toAscii(), QCryptographicHash::Md5).toHex();
    url.addQueryItem("api_sig", hash);

    req.setUrl(url);
    req.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    QNetworkReply* reply = m_pNetwork->post(req, body);
    bool res = QObject::connect(reply, SIGNAL(finished()), this, SLOT(onAuthenticate()));
    Q_ASSERT(res);
    res = QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

void LastFMController::onAuthenticate() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());

    if (reply->error() == QNetworkReply::NoError) {
        JsonDataAccess jda;
        QVariantMap session = jda.loadFromBuffer(reply->readAll()).toMap().value("session").toMap();
        QString name = session.value("name").toString();
        QString key = session.value("key").toString();

        m_pAppConfig->set(LAST_FM_NAME, name);
        m_pAppConfig->set(LAST_FM_KEY, key);
        emit authenticationFinished(tr("Logged in as ") + name, true);
    }

    reply->deleteLater();
}

void LastFMController::onError(QNetworkReply::NetworkError e) {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    qDebug() << "===>>> LastFMController#onError " << e << endl;
    qDebug() << "===>>> LastFMController#onError " << reply->errorString() << endl;
    emit authenticationFinished(reply->errorString(), false);
    reply->deleteLater();
}

TrackController* LastFMController::getTrackController() const {
    return m_pTrack;
}
