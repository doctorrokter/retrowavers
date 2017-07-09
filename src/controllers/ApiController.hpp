/*
 * ApiController.hpp
 *
 *  Created on: Jun 24, 2017
 *      Author: misha
 */

#ifndef APICONTROLLER_HPP_
#define APICONTROLLER_HPP_

#include <QtCore/QObject>
#include <QVariantList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "../services/TracksService.hpp"
#include <bb/system/SystemToast>

using namespace bb::system;

class ApiController: public QObject {
    Q_OBJECT
public:
    ApiController(TracksService* tracks, QObject* parent = 0);
    virtual ~ApiController();

    Q_INVOKABLE void load();
    void loadImage(const QString& id, const QString& path);
    void loadBlurImage(const QString& id, const QString& path);

    Q_SIGNALS:
        void loaded(const QVariantList& songs);

private slots:
    void onLoad();
    void onLoadError(QNetworkReply::NetworkError e);
    void onImageLoad();
    void onBlurImageLoad();
    void onImageError(QNetworkReply::NetworkError e);

private:
    QNetworkAccessManager* m_network;
    int m_cursor;

    TracksService* m_tracks;
    SystemToast* m_pToast;
};

#endif /* APICONTROLLER_HPP_ */
