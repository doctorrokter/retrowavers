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

#include <QLocale>
#include <QTranslator>
#include <QtCore/QTimer>
#include <Qt/qdeclarativedebug.h>
#include "models/Track.hpp"
#include "vendor/Console.hpp"
#include <QFile>
#include <QSslSocket>
#include <QDebug>
#include "Common.hpp"

using namespace bb::cascades;

// Startup trace, DEVELOPMENT builds only (RW_PRODUCTION comes from the BBT_ENV
// block in Retrowavers.pro). The log has to leave the device as a FILE:
// shared/misc is reachable from the file manager and over USB, and ssh on BB10
// only lives while a blackberry-connect session is held. The extension is .txt
// ON PURPOSE - the stock BB10 viewer opens txt and refuses .log, so a log nobody
// can read on the phone is no log at all. Append-only, and no qDebug inside:
// that would recurse through the message handler this is called from.
static void logToFile(const char* msg) {
#ifdef RW_PRODUCTION
    Q_UNUSED(msg);
#else
    QFile file("/accounts/1000/shared/misc/retrowavers-start.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        file.write(msg);
        file.write("\n");
        file.close();
    }
#endif
}

void myMessageOutput(QtMsgType type, const char* msg) {  // <-- ADD THIS
    Q_UNUSED(type);
    fprintf(stdout, "%s\n", msg);
    fflush(stdout);
    logToFile(msg);

    QSettings settings;
    if (settings.value("sendToConsoleDebug", true).toBool()) {
        Console* console = new Console();
        console->sendMessage("ConsoleThis$$" + QString(msg));
        console->deleteLater();
    }
}


// The BB10 trust store predates every root in use today, so our API (a Let's Encrypt
// chain) fails the handshake with QNetworkReply::SslHandshakeFailedError until Qt is
// handed its own roots. QSslSocket's default CA set belongs to the process, not to any
// one object, which is why this lives in main() rather than in a constructor.
// NOTE: this covers Qt requests ONLY - mm-renderer fetches the audio stream itself and
// never sees these certificates.
static void installCaBundle() {
    QString bundle = "app/native/assets/certs/ca-bundle.pem";
    if (!QFile::exists(bundle)) {
        qDebug() << "===>>> installCaBundle: no bundle at " << bundle << endl;
        return;
    }

    bool ok = QSslSocket::addDefaultCaCertificates(bundle);
    qDebug() << "===>>> installCaBundle: loaded " << ok << " total roots: " << QSslSocket::defaultCaCertificates().size() << endl;
}

Q_DECL_EXPORT int main(int argc, char **argv) {
    qmlRegisterType<QTimer>("chachkouski.util", 1, 0, "Timer");
    qmlRegisterType<TracksController>("chachkouski.enums", 1, 0, "PlayerMode");
    qRegisterMetaType<Track*>("Track*");
    qmlRegisterUncreatableType<Track>("chachkouski.type", 1, 0, "track", "test");

    qInstallMsgHandler(myMessageOutput);
    RW_TRACE("===>>> STEP 0: main() entered");

    Application app(argc, argv);
    RW_TRACE("===>>> STEP 1: Application created");
    installCaBundle();
    RW_TRACE("===>>> STEP 2: CA bundle done");
    ApplicationUI appui;
    RW_TRACE("===>>> STEP 9: ApplicationUI done");
    return Application::exec();
}
