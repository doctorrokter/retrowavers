/*
 * RadioSource.cpp - see RadioSource.hpp.
 */

#include "RadioSource.hpp"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>
#include <QStringList>
#include <QDebug>

// Tracks run three to six minutes, so half a minute is frequent enough to look live
// without hammering anyone's server.
static const int kPollMs = 30000;

RadioSource::RadioSource(QObject* parent) : ITrackSource(parent),
        m_network(new QNetworkAccessManager(this)), m_poll(new QTimer(this)), m_delivered(false) {
    m_poll->setInterval(kPollMs);
    bool res = QObject::connect(m_poll, SIGNAL(timeout()), this, SLOT(onPollTimeout()));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

RadioSource::~RadioSource() {
    m_poll->stop();
    m_network->deleteLater();
}

QString RadioSource::kind() const { return "radio"; }
bool RadioSource::canList() const { return true; }
bool RadioSource::canDownload() const { return false; }

QNetworkAccessManager* RadioSource::network() const { return m_network; }
QString RadioSource::activeTrackId() const { return m_activeTrackId; }

void RadioSource::reset() {
    m_delivered = false;
    m_stations.clear();
    m_activeTrackId = "";
    m_poll->stop();
}

void RadioSource::loadMore() {
    if (m_delivered) {
        // A station list is complete the moment it arrives: there is no next page.
        // Answer anyway - the list view keeps its spinner up until we do.
        emit tracksLoaded(QVariantList());
        return;
    }
    requestStations();
}

void RadioSource::deliverStations(const QList<Station>& stations) {
    m_stations = stations;
    m_delivered = true;

    QVariantList tracks;
    for (int i = 0; i < stations.size(); ++i) {
        tracks.append(toTrackMap(stations.at(i)));
    }

    qDebug() << "===>>> " << id() << " stations: " << tracks.size() << endl;
    emit tracksLoaded(tracks);
}

QVariantMap RadioSource::toTrackMap(const Station& station) const {
    QVariantMap map;

    map["id"] = id() + ":" + station.key;
    map["title"] = station.title;
    map["author"] = name();
    map["name"] = station.title;
    map["tags"] = QStringList() << station.genre;

    // A live stream has no length; the UI hides the timer when this is 0.
    map["duration"] = 0;

    map["streamUrl"] = station.streamUrl;
    map["artworkUrl"] = station.artworkUrl;
    map["bArtworkUrl"] = "";
    map["favourite"] = false;
    map["filename"] = "";

    return map;
}

QString RadioSource::stationKeyOf(const QString& trackId) const {
    const QString prefix = id() + ":";
    return trackId.startsWith(prefix) ? trackId.mid(prefix.length()) : QString("");
}

QString RadioSource::joinArtistTitle(const QString& artist, const QString& title) {
    if (artist.isEmpty()) {
        return title;
    }
    // The en dash is built from its code point rather than pasted in: it is a data
    // format (the catalogue uses it and the player splits on it), not prose.
    return artist + " " + QString(QChar(0x2013)) + " " + title;
}

void RadioSource::setActiveTrack(const QString& trackId) {
    if (stationKeyOf(trackId).isEmpty()) {
        // Something else is playing - another source, or nothing at all.
        m_activeTrackId = "";
        m_poll->stop();
        return;
    }

    if (m_activeTrackId == trackId) {
        return;
    }

    m_activeTrackId = trackId;
    requestNowPlaying(stationKeyOf(trackId));   // don't wait a full interval for the first song
    m_poll->start();
}

void RadioSource::onPollTimeout() {
    const QString key = stationKeyOf(m_activeTrackId);
    if (!key.isEmpty()) {
        requestNowPlaying(key);
    }
}

QString RadioSource::nowPlayingUrl(const QString& stationKey) const {
    Q_UNUSED(stationKey);
    return QString("");
}

bool RadioSource::parseNowPlaying(const QByteArray& body, const QString& stationKey,
                                  QVariantMap& fields) const {
    Q_UNUSED(body);
    Q_UNUSED(stationKey);
    Q_UNUSED(fields);
    return false;
}

void RadioSource::requestNowPlaying(const QString& stationKey) {
    const QString url = nowPlayingUrl(stationKey);
    if (url.isEmpty()) {
        return;   // this service does not report what is playing
    }

    QNetworkRequest req;
    req.setUrl(QUrl(url));

    QNetworkReply* reply = m_network->get(req);
    reply->setProperty("trackId", m_activeTrackId);
    reply->setProperty("stationKey", stationKey);
    bool res = QObject::connect(reply, SIGNAL(finished()), this, SLOT(onNowPlayingReply()));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

void RadioSource::onNowPlayingReply() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    if (reply == NULL) {
        return;
    }

    const QString trackId = reply->property("trackId").toString();
    const QString stationKey = reply->property("stationKey").toString();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "===>>> " << id() << " now-playing error: " << reply->errorString() << endl;
        reply->deleteLater();
        return;
    }

    QVariantMap fields;
    if (parseNowPlaying(reply->readAll(), stationKey, fields)) {
        publishNowPlaying(trackId, fields);
    }

    reply->deleteLater();
}

void RadioSource::publishNowPlaying(const QString& trackId, const QVariantMap& fields) {
    if (trackId != m_activeTrackId) {
        return;   // the station changed while the request was in flight
    }

    qDebug() << "===>>> " << id() << " on air: " << fields.value("title").toString() << endl;
    emit trackUpdated(trackId, fields);
}
