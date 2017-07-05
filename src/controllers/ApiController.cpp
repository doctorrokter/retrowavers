/*
 * ApiController.cpp
 *
 *  Created on: Jun 24, 2017
 *      Author: misha
 */

#include "ApiController.hpp"
#include <QNetworkRequest>
#include <QUrl>
#include <bb/data/JsonDataAccess>
#include <QVariantMap>
#include <QDebug>
#include <QVariantList>
#include <QByteArray>
#include <QFile>
#include <QDir>
#include <QList>
#include "../Common.hpp"
#include "../models/Track.hpp"

using namespace bb::data;

ApiController::ApiController(TracksService* tracks, QObject* parent) : QObject(parent), m_network(new QNetworkAccessManager(this)), m_cursor(1), m_tracks(tracks) {
    m_pToast = new SystemToast(this);
}

ApiController::~ApiController() {
    m_network->deleteLater();
    m_pToast->deleteLater();
}

void ApiController::load() {
    QNetworkRequest req;

    QUrl url(QString(API_ENDPOINT).append("/tracks"));
    url.addQueryItem("cursor", QString::number(m_cursor));
    url.addQueryItem("limit", QString::number(25));

    req.setUrl(url);

    qDebug() << "===>>> ApiController#load " << url << endl;

    QNetworkReply* reply = m_network->get(req);
    bool res = QObject::connect(reply, SIGNAL(finished()), this, SLOT(onLoad()));
    Q_ASSERT(res);
    res = QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onLoadError(QNetworkReply::NetworkError)));
    Q_UNUSED(res);
}

void ApiController::onLoad() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());

    if (reply->error() == QNetworkReply::NoError) {
        JsonDataAccess jda;
            QVariantMap response = jda.loadFromBuffer(reply->readAll()).toMap();
            QVariantMap body = response.value("body").toMap();
            m_cursor = body.value("cursor").toInt();

            QVariantList tracks = body.value("tracks").toList();
            QVariantList updatedTracks;
            QList<Track*> tracksList;

            foreach(QVariant var, tracks) {
                QVariantMap trMap = var.toMap();
                Track* track = m_tracks->findFavouriteById(trMap.value("id").toString());
                if (track == NULL) {
                    QString imageUrl = trMap.value("artworkUrl").toString();
                    trMap["artworkUrl"] = QString(ROOT_IMAGE_ENDPOINT).append(imageUrl);

                    if (imageUrl.endsWith(".png", Qt::CaseInsensitive)) {
                        trMap["b_artworkUrl"] = QString(ROOT_IMAGE_ENDPOINT).append(".rsz.io").append(imageUrl).append("?blur=65");
                    } else {
                        trMap["b_artworkUrl"] = QString("https://i.scaley.io/b65/retrowave.ru").append(imageUrl);
                    }
                    trMap["streamUrl"] = ROOT_ENDPOINT + trMap.value("streamUrl").toString();
                    trMap["favourite"] = false;

                    QString filename = trMap.value("streamUrl").toString().split("/").last();
                    trMap["filename"] = filename;

                    updatedTracks.append(trMap);

                    track = new Track(this);
                    track->fromMap(trMap);
                } else {
                    qDebug() << "===>>> ApiController#onLoad track already favourite: " << track->getTitle() << endl;
                    qDebug() << track->toMap() << endl;
                }
                tracksList.append(track);
            }

            m_tracks->appendTracks(tracksList);
            foreach(Track* track, tracksList) {
                loadImage(track->getId(), track->getArtworkUrl());
                loadBlurImage(track->getId(), track->getBArtworkUrl());
            }

            emit loaded(updatedTracks);
    }

    reply->deleteLater();
}

void ApiController::onLoadError(QNetworkReply::NetworkError e) {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    m_pToast->setBody(tr("Service is unavailable. Try later."));
    m_pToast->show();
    qDebug() << "===>>> ApiController#onLoadError: " << e << endl;
    qDebug() << "===>>> ApiController#onLoadError: " << reply->errorString() << endl;
}

void ApiController::loadImage(const QString& id, const QString& path) {
    QNetworkRequest req;

    QUrl url(path);
    req.setUrl(url);

    QString filename = path.split("/").last();
    QString filepath = QDir::currentPath() + IMAGES + "/" + filename;
    QFile file(filepath);
    if (file.exists()) {
        qDebug() << "===>>> ApiController#loadImage - file exists: " << filepath << endl;
        m_tracks->setImagePath(id, filepath);
    } else {
        QNetworkReply* reply = m_network->get(req);
        reply->setProperty("id", id);
        reply->setProperty("path", path);
        reply->setProperty("filename", filename);
        reply->setProperty("filepath", filepath);
        bool res = QObject::connect(reply, SIGNAL(finished()), this, SLOT(onImageLoad()));
        Q_ASSERT(res);
        res = QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onImageError(QNetworkReply::NetworkError)));
        Q_UNUSED(res);
    }
}

void ApiController::onImageLoad() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    QString id = reply->property("id").toString();
    QString path = reply->property("path").toString();
    QString filename = reply->property("filename").toString();
    QString filepath = reply->property("filepath").toString();

    QByteArray data = reply->readAll();

    if (data.size() != 0 && reply->error() == QNetworkReply::NoError) {
        QString imagesPath = QDir::currentPath() + IMAGES;
        QDir images(imagesPath);
        if (!images.exists()) {
            images.mkpath(imagesPath);
        }

        QString imagePath = filepath;
        QFile image(imagePath);
        if (image.open(QIODevice::WriteOnly)) {
            image.write(data);
            image.close();
            m_tracks->setImagePath(id, imagePath);
        } else {
            qDebug() << "===>>> ERROR OPEN FILE: " << filepath << " " << image.errorString() << endl;
        }
    }

    reply->deleteLater();
}

void ApiController::onImageError(QNetworkReply::NetworkError e) {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    qDebug() << "ApiController#onError: " << e << endl;
    qDebug() << "ApiController#onError: " << reply->errorString() << endl;
    qDebug() << "ApiController#onError: " << reply->property("path").toString() << endl;
    reply->deleteLater();
}

void ApiController::loadBlurImage(const QString& id, const QString& path) {
    QNetworkRequest req;

    QUrl url(path);
    req.setUrl(url);

    QString filename = "b_" + (path.split("/").last().split("?").first());
    QString filepath = QDir::currentPath() + IMAGES + "/" + filename;
    QFile file(filepath);
    if (file.exists()) {
        qDebug() << "===>>> ApiController#loadBlurImage - file exists: " << filepath << endl;
        m_tracks->setBlurImagePath(id, filepath);
    } else {
        QNetworkReply* reply = m_network->get(req);
        reply->setProperty("id", id);
        reply->setProperty("path", path);
        reply->setProperty("filename", filename);
        reply->setProperty("filepath", filepath);
        bool res = QObject::connect(reply, SIGNAL(finished()), this, SLOT(onBlurImageLoad()));
        Q_ASSERT(res);
        res = QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onImageError(QNetworkReply::NetworkError)));
        Q_UNUSED(res);
    }
}

void ApiController::onBlurImageLoad() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    QString id = reply->property("id").toString();
    QString path = reply->property("path").toString();
    QString filename = reply->property("filename").toString();
    QString filepath = reply->property("filepath").toString();

    QByteArray data = reply->readAll();

    if (data.size() != 0  && reply->error() == QNetworkReply::NoError) {
        QString imagesPath = QDir::currentPath() + IMAGES;
        QDir images(imagesPath);
        if (!images.exists()) {
            images.mkpath(imagesPath);
        }

        QString imagePath = filepath;
        QFile image(imagePath);
        image.open(QIODevice::WriteOnly);
        image.write(data);
        image.close();
        m_tracks->setBlurImagePath(id, imagePath);
    }
    reply->deleteLater();
}
