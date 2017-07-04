/*
 * TracksController.hpp
 *
 *  Created on: Jun 24, 2017
 *      Author: misha
 */

#ifndef TRACKSCONTROLLER_HPP_
#define TRACKSCONTROLLER_HPP_

#include <QObject>
#include <QVariantMap>
#include "../services/TracksService.hpp"
#include <bb/platform/Notification>
#include <bb/platform/NotificationType>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <bb/system/SystemToast>

using namespace bb::platform;
using namespace bb::system;

class TracksController: public QObject {
    Q_OBJECT
public:
    TracksController(TracksService* tracks, QObject* parent = 0);
    virtual ~TracksController();

    Q_INVOKABLE bool play(const QVariantMap& track);
    Q_INVOKABLE bool next();
    Q_INVOKABLE bool prev();
    Q_INVOKABLE void like();

    Q_SIGNALS:
        void played(const QVariantMap& track);
        void liked(const QString& id);
        void downloaded(const QString& id);

private slots:
    void onDownload();
    void onDownloadError(QNetworkReply::NetworkError e);
    void onDownloadProgress(qint64 sent, qint64 total);

private:
    TracksService* m_tracks;
    int m_index;

    Notification* m_pNotification;
    QNetworkAccessManager* m_pNetwork;
    SystemToast* m_pToast;

    void notify(Track* track);
    void download(Track* track);
};

#endif /* TRACKSCONTROLLER_HPP_ */
