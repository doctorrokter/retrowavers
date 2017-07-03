/*
 * TracksController.cpp
 *
 *  Created on: Jun 24, 2017
 *      Author: misha
 */

#include "TracksController.hpp"
#include "../Common.hpp"
#include <QNetworkRequest>
#include <QUrl>
#include <QDir>
#include <QFile>

#define NOTIFICATION_KEY "Retrowavers"

TracksController::TracksController(TracksService* tracks, QObject* parent) : QObject(parent), m_tracks(tracks), m_index(0) {
    m_pNotification = new Notification(this);
    m_pNotification->setTitle(NOTIFICATION_KEY);
    m_pNotification->setType(NotificationType::AllAlertsOff);
    m_pNotification->deleteAllFromInbox();

    m_pNetwork = new QNetworkAccessManager(this);
}

TracksController::~TracksController() {
    m_pNotification->deleteAllFromInbox();
    m_pNotification->deleteLater();
    m_pNetwork->deleteLater();
}

void TracksController::play(const QVariantMap& track) {
    m_index = 0;
    if (track.contains("id")) {
        for (int i = 0; i < m_tracks->count(); i++) {
            QString id = track.value("id").toString();
            if (m_tracks->getTracksList().at(i)->getId().compare(id) == 0) {
                m_index = i;

                Track* pTrack = m_tracks->findById(id);
                m_tracks->setActive(pTrack);
                notify(pTrack);
                emit played(pTrack->toMap());
            }
        }
    } else {
        Track* pTrack = m_tracks->getTracksList().at(m_index);
        m_tracks->setActive(pTrack);
        notify(pTrack);
        emit played(pTrack->toMap());
    }
}

bool TracksController::next() {
    if (m_index < (m_tracks->count() -1)) {
        m_index++;
        play(m_tracks->getTracks().at(m_index).toMap());
        return true;
    }
    return false;
}

bool TracksController::prev() {
    if (m_index != 0) {
        m_index--;
        play(m_tracks->getTracks().at(m_index).toMap());
        return true;
    }
    return false;
}

void TracksController::like() {
    Track* track = m_tracks->getActive();
    if (track != NULL && !track->isFavourite()) {
        track->setFavourite(true);
        m_tracks->addFavourite(track);
        download(track);
        emit liked(track->getId());
    }
}

void TracksController::notify(Track* track) {
    m_pNotification->setBody(track->getTitle());
    m_pNotification->notify();
}

void TracksController::download(Track* track) {
    QNetworkRequest req;

    QUrl url(track->getStreamUrl());
    req.setUrl(url);

    QNetworkReply* reply = m_pNetwork->get(req);
    reply->setProperty("id", track->getId());
    bool res = QObject::connect(reply, SIGNAL(finished()), this, SLOT(onDownload()));
    Q_ASSERT(res);
    res = QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onDownloadError(QNetworkReply::NetworkError)));
    Q_ASSERT(res);
    res = QObject::connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onDownloadProgress(qint64, qint64)));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

void TracksController::onDownload() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    QString id = reply->property("id").toString();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();

        QString tracksDir = QDir::currentPath() + TRACKS;
        QDir dir(tracksDir);
        if (!dir.exists()) {
            dir.mkpath(tracksDir);
        }

        Track* track = m_tracks->findById(id);
        QString filepath = tracksDir + "/" + track->getFilename();
        QFile file(filepath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(data);
            file.close();
            track->setLocalPath(filepath);
        } else {
            qDebug() << file.errorString() << endl;
        }
    }

    reply->deleteLater();
}

void TracksController::onDownloadError(QNetworkReply::NetworkError e) {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    qDebug() << "TracksController#onError: " << e << endl;
    qDebug() << "TracksController#onError: " << reply->errorString() << endl;
    reply->deleteLater();
}

void TracksController::onDownloadProgress(qint64 sent, qint64 total) {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    QString trackId = reply->property("id").toString();
    qDebug() << "SENT: " << sent << " TOTAL: " << total << endl;
}
