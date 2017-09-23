/*
 * FacebookController.hpp
 *
 *  Created on: Aug 15, 2017
 *      Author: misha
 */

#ifndef FACEBOOKCONTROLLER_HPP_
#define FACEBOOKCONTROLLER_HPP_

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVariantMap>
#include <bb/system/SystemToast>

using namespace bb::system;

class FacebookController: public QObject {
    Q_OBJECT
public:
    FacebookController(QObject* parent = 0);
    virtual ~FacebookController();

    Q_INVOKABLE void share(const QVariantMap& track, const QString& message) const;

    Q_SIGNALS:
        void shared();

private slots:
    void onShare();
    void onError(QNetworkReply::NetworkError e);

private:
    QNetworkAccessManager* m_pNetwork;
    SystemToast* m_pToast;
};

#endif /* FACEBOOKCONTROLLER_HPP_ */
