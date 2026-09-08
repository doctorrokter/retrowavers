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
    RW_TRACE("===>>> STEP 3: AppConfig");
    m_pToast = new SystemToast(this);

    m_pNetworkConf = new QNetworkConfigurationManager(this);
    m_online = m_pNetworkConf->isOnline();

    m_tracks = new TracksService(this);
    RW_TRACE("===>>> STEP 4: TracksService");
    m_tracksController = new TracksController(m_tracks, this);
    RW_TRACE("===>>> STEP 5: TracksController");
    m_api = new ApiController(m_tracks, this);
    migrateFromLegacy();
    RW_TRACE("===>>> STEP 6: ApiController");

    bool res = QObject::connect(m_pLocaleHandler, SIGNAL(systemLanguageChanged()), this, SLOT(onSystemLanguageChanged()));
    Q_ASSERT(res);
    res = QObject::connect(m_pNetworkConf, SIGNAL(onlineStateChanged(bool)), this, SLOT(onOnlineChanged(bool)));
    Q_ASSERT(res);
    Q_UNUSED(res);

    onSystemLanguageChanged();

    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);
    RW_TRACE("===>>> STEP 7: QmlDocument");
    QDeclarativeEngine* engine = QmlDocument::defaultDeclarativeEngine();
    QDeclarativeContext* rootContext = engine->rootContext();
    rootContext->setContextProperty("_app", this);
    rootContext->setContextProperty("_api", m_api);
    rootContext->setContextProperty("_tracksService", m_tracks);
    rootContext->setContextProperty("_tracksController", m_tracksController);
    rootContext->setContextProperty("_appConfig", m_pAppConfig);

    AbstractPane *root = qml->createRootObject<AbstractPane>();
    RW_TRACE("===>>> STEP 8: root object");
    Application::instance()->setScene(root);
}

ApplicationUI::~ApplicationUI() {
    m_pTranslator->deleteLater();
    m_pLocaleHandler->deleteLater();

    m_api->deleteLater();
    m_tracks->deleteLater();
    m_tracksController->deleteLater();
    m_pAppConfig->deleteLater();
    m_pNetworkConf->deleteLater();
    m_pToast->deleteLater();
}

// Upgrading from 2.x leaves favourites pointing at a dead host and a cache full of
// blurs fetched from a web service that no longer exists. Runs once, then never again.
void ApplicationUI::migrateFromLegacy() {
    if (!m_pAppConfig->get("migrated_v3").toString().isEmpty()) {
        return;
    }

    int migrated = m_tracks->migrateLegacyFavourites();
    removeLegacyBlurs(QDir::currentPath() + IMAGES);
    m_pAppConfig->set("migrated_v3", "true");

    qDebug() << "===>>> migrateFromLegacy: favourites migrated: " << migrated << endl;
}

// The old blurs were named "b_<something>.png"; nothing reads them any more, and the
// on-device renderer uses its own names (<uuid>_b160_22.png), so these are dead weight.
void ApplicationUI::removeLegacyBlurs(const QString& path) {
    QDir dir(path);
    if (!dir.exists()) {
        return;
    }

    QFileInfoList list = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
    Q_FOREACH(QFileInfo info, list) {
        if (info.fileName().startsWith("b_")) {
            QFile::remove(info.absoluteFilePath());
        }
    }
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
