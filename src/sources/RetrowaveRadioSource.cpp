/*
 * RetrowaveRadioSource.cpp - see RetrowaveRadioSource.hpp.
 */

#include "RetrowaveRadioSource.hpp"
#include "../Common.hpp"

#include <bb/data/JsonDataAccess>

#include <QNetworkRequest>
#include <QUrl>
#include <QUuid>
#include <QStringList>
#include <QDebug>

using namespace bb::data;

RetrowaveRadioSource::RetrowaveRadioSource(QObject* parent) : ITrackSource(parent),
        m_network(new QNetworkAccessManager(this)), m_inFlight(false) {
    reset();
}

RetrowaveRadioSource::~RetrowaveRadioSource() {
    m_network->deleteLater();
}

QString RetrowaveRadioSource::id() const { return "rwr"; }
QString RetrowaveRadioSource::name() const { return "RetroWave Radio"; }
QString RetrowaveRadioSource::kind() const { return "catalog"; }

bool RetrowaveRadioSource::canList() const { return true; }
bool RetrowaveRadioSource::canDownload() const { return true; }

void RetrowaveRadioSource::reset() {
    // The server keys "what have I already given you" off this id; a fresh one
    // means a fresh shuffle of the whole catalogue. Braces are stripped because
    // they would have to be percent-encoded in the query for no gain.
    m_sessionId = QUuid::createUuid().toString().remove("{").remove("}");
    m_seen.clear();
    qDebug() << "===>>> RetrowaveRadioSource#reset session: " << m_sessionId << endl;
}

void RetrowaveRadioSource::loadMore() {
    if (m_inFlight) {
        // ListScrollStateHandler fires atEnd repeatedly while the list bounces,
        // and the player asks for more when it runs out - without this guard one
        // scroll turns into a handful of identical requests.
        return;
    }
    m_inFlight = true;

    QUrl url(QString(RWR_API_ENDPOINT).append("/tracks/shuffled"));
    url.addQueryItem("limit", QString::number(RWR_PAGE_LIMIT));
    url.addQueryItem("sessionId", m_sessionId);

    QNetworkRequest req;
    req.setUrl(url);

    qDebug() << "===>>> RetrowaveRadioSource#loadMore " << url << endl;

    QNetworkReply* reply = m_network->get(req);
    bool res = QObject::connect(reply, SIGNAL(finished()), this, SLOT(onTracksReply()));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

void RetrowaveRadioSource::onTracksReply() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    m_inFlight = false;

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "===>>> RetrowaveRadioSource#onTracksReply error: " << reply->error() << " " << reply->errorString() << endl;
        emit loadFailed(reply->errorString());
        reply->deleteLater();
        return;
    }

    JsonDataAccess jda;
    QVariantList json = jda.loadFromBuffer(reply->readAll()).toList();

    QVariantList tracks;
    foreach(QVariant var, json) {
        QVariantMap map = toTrackMap(var.toMap());
        QString trackId = map.value("id").toString();
        if (m_seen.contains(trackId)) {
            continue;   // the catalogue wrapped around - the server repeats itself
        }
        m_seen.insert(trackId);
        tracks.append(map);
    }

    qDebug() << "===>>> RetrowaveRadioSource#onTracksReply got " << json.size() << " new " << tracks.size() << " seen " << m_seen.size() << endl;

    // An empty list is emitted too: whoever asked is waiting for an answer.
    emit tracksLoaded(tracks);
    reply->deleteLater();
}

QVariantMap RetrowaveRadioSource::toTrackMap(const QVariantMap& json) const {
    QVariantMap map;

    QString streamPath = json.value("streamUrl").toString();
    QString artworkPath = json.value("artworkUrl").toString();
    QString author = json.value("author").toString();
    QString name = json.value("name").toString();

    map["id"] = id() + ":" + json.value("id").toString();

    // One display string out of two fields. The separator is an en dash (0x2013)
    // built from its code point rather than pasted into the source: it is data
    // format, not prose - the old API used it and Player.qml splits on it to get
    // artist and track for the "now playing" overlay.
    map["title"] = author + " " + QString(QChar(0x2013)) + " " + name;
    map["author"] = author;
    map["name"] = name;
    map["tags"] = json.value("tags").toStringList();

    map["duration"] = json.value("duration").toInt();
    map["streamUrl"] = QString(RWR_ROOT_ENDPOINT).append(streamPath);
    map["artworkUrl"] = QString(RWR_ROOT_ENDPOINT).append(artworkPath);
    map["bArtworkUrl"] = "";   // the blur is rendered on the device now, not fetched
    map["favourite"] = false;

    // "<uuid>.mp3" - unique per track, which is what the download path relies on.
    map["filename"] = streamPath.split("/").last();

    return map;
}
