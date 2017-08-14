/*
 * VKController.hpp
 *
 *  Created on: Aug 14, 2017
 *      Author: misha
 */

#ifndef VKCONTROLLER_HPP_
#define VKCONTROLLER_HPP_

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVariantMap>
#include <bb/system/SystemToast>

using namespace bb::system;

class VKController: public QObject {
    Q_OBJECT
public:
    VKController(QObject* parent = 0);
    virtual ~VKController();

    Q_INVOKABLE void share(const QVariantMap& track) const;

    Q_SIGNALS:
        void shared(const QVariantMap& track);

private slots:
        void onShare();
        void onError(QNetworkReply::NetworkError e);

private:
    QNetworkAccessManager* m_pNetwork;
    SystemToast* m_pToast;
};

#endif /* VKCONTROLLER_HPP_ */
