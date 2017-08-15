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

#include "applicationui.hpp"

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/LocaleHandler>
#include <QDir>
#include <QFile>
#include "Common.hpp"

using namespace bb::cascades;

ApplicationUI::ApplicationUI() : QObject() {
    m_pTranslator = new QTranslator(this);
    m_pLocaleHandler = new LocaleHandler(this);

    QDir dir(QDir::currentPath() + IMAGES);
    if (!dir.exists(QDir::currentPath() + IMAGES)) {
        dir.mkdir(QDir::currentPath() + IMAGES);
    }

    m_pAppConfig = new AppConfig(this);
    m_pToast = new SystemToast(this);

    m_pNetworkConf = new QNetworkConfigurationManager(this);
    m_online = m_pNetworkConf->isOnline();

    m_tracks = new TracksService(this);
    m_tracksController = new TracksController(m_tracks, this);
    m_pVKController = new VKController(this);
    m_pFBController = new FacebookController(this);
    m_lastFM = new LastFMController(m_pAppConfig, this);
    m_api = new ApiController(m_tracks, this);

    if (m_pAppConfig->get("changed_image_processor").toString().isEmpty()) {
        cleanDir(QDir::currentPath() + IMAGES);
        foreach(Track* track, m_tracks->getFavouriteTracksList()) {
            track->setBArtworkUrl(QString(IMAGE_PROCESSOR_URL).append("/blur/65/").append(track->getArtworkUrl()));
            track->setBImagePath("");
            qDebug() << "Download blur: " << track->getBArtworkUrl() << endl;
            m_api->loadBlurImage(track->getId(), track->getBArtworkUrl());
        }
        m_pAppConfig->set("changed_image_processor", true);
    }

    bool res = QObject::connect(m_pLocaleHandler, SIGNAL(systemLanguageChanged()), this, SLOT(onSystemLanguageChanged()));
    Q_ASSERT(res);
    res = QObject::connect(m_pNetworkConf, SIGNAL(onlineStateChanged(bool)), this, SLOT(onOnlineChanged(bool)));
    Q_ASSERT(res);
    Q_UNUSED(res);

    onSystemLanguageChanged();

    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);
    QDeclarativeEngine* engine = QmlDocument::defaultDeclarativeEngine();
    QDeclarativeContext* rootContext = engine->rootContext();
    rootContext->setContextProperty("_app", this);
    rootContext->setContextProperty("_api", m_api);
    rootContext->setContextProperty("_tracksService", m_tracks);
    rootContext->setContextProperty("_tracksController", m_tracksController);
    rootContext->setContextProperty("_vkController", m_pVKController);
    rootContext->setContextProperty("_fbController", m_pFBController);
    rootContext->setContextProperty("_lastFM", m_lastFM);
    rootContext->setContextProperty("_appConfig", m_pAppConfig);

    AbstractPane *root = qml->createRootObject<AbstractPane>();
    Application::instance()->setScene(root);
}

ApplicationUI::~ApplicationUI() {
    m_pTranslator->deleteLater();
    m_pLocaleHandler->deleteLater();

    m_api->deleteLater();
    m_tracks->deleteLater();
    m_tracksController->deleteLater();
    m_pFBController->deleteLater();
    m_pVKController->deleteLater();
    m_lastFM->deleteLater();
    m_pAppConfig->deleteLater();
    m_pNetworkConf->deleteLater();
    m_pToast->deleteLater();
}

void ApplicationUI::onSystemLanguageChanged() {
    QCoreApplication::instance()->removeTranslator(m_pTranslator);
    QString locale_string = QLocale().name();
    QString file_name = QString("Retrowavers_%1").arg(locale_string);
    if (m_pTranslator->load(file_name, "app/native/qm")) {
        QCoreApplication::instance()->installTranslator(m_pTranslator);
    }
}

void ApplicationUI::toast(const QString& message) {
    m_pToast->setBody(message);
    m_pToast->show();
}

bool ApplicationUI::isOnline() const { return m_online; }
void ApplicationUI::onOnlineChanged(bool online) {
    if (m_online != online) {
        m_online = online;
        emit onlineChanged(m_online);
    }
}

void ApplicationUI::cleanDir(const QString& path) {
    QDir dir(path);

    qDebug() << "Clean dir: " << path << endl;

    if (dir.exists(path)) {
        QFileInfoList list = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst);
        Q_FOREACH(QFileInfo info, list) {
            if (info.isDir()) {
                cleanDir(info.absoluteFilePath());
            } else {
                if (info.fileName().startsWith("b_")) {
                    qDebug() << "Remove file: " << info.fileName() << endl;
                    QFile::remove(info.absoluteFilePath());
                }
            }
        }
    }
}
