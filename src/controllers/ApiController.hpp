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

class ApiController: public QObject {
    Q_OBJECT
public:
    ApiController(TracksService* tracks, QObject* parent = 0);
    virtual ~ApiController();

    Q_INVOKABLE void load();

    Q_SIGNALS:
        void loaded(const QVariantList& songs);

private slots:
    void onLoad();
    void onImageLoad();
    void onBlurImageLoad();

private:
    QNetworkAccessManager* m_network;
    int m_cursor;

    TracksService* m_tracks;

    void loadImage(const QString& id, const QString& path);
    void loadBlurImage(const QString& id, const QString& path);
};

#endif /* APICONTROLLER_HPP_ */