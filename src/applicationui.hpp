/*
 * Copyright (c) 2011-2015 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ApplicationUI_HPP_
#define ApplicationUI_HPP_

#include <QObject>
#include "controllers/ApiController.hpp"
#include "controllers/TracksController.hpp"
#include "controllers/VKController.hpp"
#include "controllers/FacebookController.hpp"
#include "controllers/lastfm/LastFMController.hpp"
#include "services/TracksService.hpp"
#include "config/AppConfig.hpp"
#include <QNetworkConfigurationManager>
#include <bb/system/SystemToast>

namespace bb
{
    namespace cascades
    {
        class LocaleHandler;
    }
}

using namespace bb::system;

class QTranslator;

/*!
 * @brief Application UI object
 *
 * Use this object to create and init app UI, to create context objects, to register the new meta types etc.
 */
class ApplicationUI : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool online READ isOnline NOTIFY onlineChanged)
public:
    ApplicationUI();
    virtual ~ApplicationUI();

    Q_INVOKABLE void toast(const QString& message);
    Q_INVOKABLE void share(const QString& type);
    bool isOnline() const;

    Q_SIGNALS:
        void onlineChanged(const bool& online);
        void shareRequested(const QString& type);

private slots:
    void onSystemLanguageChanged();
    void onOnlineChanged(bool online);
private:
    QTranslator* m_pTranslator;
    bb::cascades::LocaleHandler* m_pLocaleHandler;

    ApiController* m_api;
    TracksController* m_tracksController;
    VKController* m_pVKController;
    FacebookController* m_pFBController;
    LastFMController* m_lastFM;
    TracksService* m_tracks;
    AppConfig* m_pAppConfig;
    QNetworkConfigurationManager* m_pNetworkConf;
    SystemToast* m_pToast;

    bool m_online;

    void cleanDir(const QString& path);
};

#endif /* ApplicationUI_HPP_ */
