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

using namespace bb::cascades;

ApplicationUI::ApplicationUI() : QObject() {
    m_pTranslator = new QTranslator(this);
    m_pLocaleHandler = new LocaleHandler(this);

    m_tracks = new TracksService(this);
    m_tracksController = new TracksController(m_tracks, this);
    m_api = new ApiController(m_tracks, this);

    bool res = QObject::connect(m_pLocaleHandler, SIGNAL(systemLanguageChanged()), this, SLOT(onSystemLanguageChanged()));
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

    AbstractPane *root = qml->createRootObject<AbstractPane>();
    Application::instance()->setScene(root);
}

void ApplicationUI::onSystemLanguageChanged() {
    QCoreApplication::instance()->removeTranslator(m_pTranslator);
    QString locale_string = QLocale().name();
    QString file_name = QString("Retrowavers_%1").arg(locale_string);
    if (m_pTranslator->load(file_name, "app/native/qm")) {
        QCoreApplication::instance()->installTranslator(m_pTranslator);
    }
}