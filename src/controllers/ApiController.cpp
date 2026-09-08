/*
 * ApiController.cpp - see ApiController.hpp.
 *
 *  Created on: Jun 24, 2017
 *      Author: misha
 */

#include "ApiController.hpp"
#include "../Common.hpp"
#include "../models/Track.hpp"
#include "../sources/RetrowaveRadioSource.hpp"
#include "../sources/SomaFmSource.hpp"
#include "../sources/NightrideSource.hpp"
#include "../sources/WaveRadioSource.hpp"
#include "../sources/PlazaSource.hpp"
#include "../sources/SynthwaveRadioSource.hpp"
#include "../sources/RetrowaveOneSource.hpp"
#include "../config/AppConfig.hpp"

#include <QNetworkRequest>
#include <QUrl>
#include <QByteArray>
#include <QFile>
#include <QDir>
#include <QVariantMap>
#include <QDebug>

ApiController::ApiController(TracksService* tracks, QObject* parent) : QObject(parent),
        m_network(new QNetworkAccessManager(this)), m_tracks(tracks), m_source(NULL) {
    m_pToast = new SystemToast(this);
    m_artwork = new ArtworkProcessor(this);

    ITrackSource* rwr = new RetrowaveRadioSource(this);
    m_sources.append(rwr);
    connectSource(rwr);

    // Radio services. Each keeps its OWN station list - the Radio tab shows the
    // services first and only then the stations of the one picked, so stations from
    // different services never end up in one flat list.
    ITrackSource* soma = new SomaFmSource(this);
    m_sources.append(soma);
    connectSource(soma);

    ITrackSource* nightride = new NightrideSource(this);
    m_sources.append(nightride);
    connectSource(nightride);

    ITrackSource* waveradio = new WaveRadioSource(this);
    m_sources.append(waveradio);
    connectSource(waveradio);

    ITrackSource* plaza = new PlazaSource(this);
    m_sources.append(plaza);
    connectSource(plaza);

    ITrackSource* synthwave = new SynthwaveRadioSource(this);
    m_sources.append(synthwave);
    connectSource(synthwave);

    ITrackSource* retrowaveOne = new RetrowaveOneSource(this);
    m_sources.append(retrowaveOne);
    connectSource(retrowaveOne);

    // Remember which source the user last listened to; the catalogue is the default.
    m_source = rwr;
    QString saved = AppConfig::getStatic("source").toString();
    foreach(ITrackSource* source, m_sources) {
        if (source->id().compare(saved) == 0) {
            m_source = source;
        }
    }

    // Artwork follows what is playing rather than what is listed - see the note
    // about image sizes in the header.
    bool res = QObject::connect(m_tracks, SIGNAL(activeChanged(Track*)), this, SLOT(onActiveChanged(Track*)));
    Q_ASSERT(res);
    res = QObject::connect(m_artwork, SIGNAL(ready(QString, QString, QString)), this, SLOT(onArtworkReady(QString, QString, QString)));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

ApiController::~ApiController() {
    m_network->deleteLater();
    m_pToast->deleteLater();
}

void ApiController::connectSource(ITrackSource* source) {
    bool res = QObject::connect(source, SIGNAL(tracksLoaded(QVariantList)), this, SLOT(onTracksLoaded(QVariantList)));
    Q_ASSERT(res);
    res = QObject::connect(source, SIGNAL(loadFailed(QString)), this, SLOT(onLoadFailed(QString)));
    Q_ASSERT(res);
    res = QObject::connect(source, SIGNAL(trackUpdated(QString, QVariantMap)), this, SLOT(onTrackUpdated(QString, QVariantMap)));
    Q_ASSERT(res);
    Q_UNUSED(res);
}

void ApiController::load() {
    if (m_source == NULL) {
        return;
    }
    m_source->loadMore();
}

QVariantList ApiController::sources() const {
    QVariantList list;
    foreach(ITrackSource* source, m_sources) {
        QVariantMap map;
        map["id"] = source->id();
        map["name"] = source->name();
        map["kind"] = source->kind();
        map["canList"] = source->canList();
        map["canDownload"] = source->canDownload();
        map["active"] = source == m_source;
        list.append(map);
    }
    return list;
}

QString ApiController::activeSource() const {
    return m_source == NULL ? QString("") : m_source->id();
}

QString ApiController::activeKind() const {
    return m_source == NULL ? QString("") : m_source->kind();
}

bool ApiController::canDownload() const {
    return m_source == NULL ? false : m_source->canDownload();
}

void ApiController::selectSource(const QString& sourceId) {
    foreach(ITrackSource* source, m_sources) {
        if (source->id().compare(sourceId) != 0 || source == m_source) {
            continue;
        }

        // The old source's tracks are gone from the list, so its polling has to stop
        // too - otherwise a radio station keeps updating a track nobody can see.
        m_source->setActiveTrack("");

        m_source = source;
        m_source->reset();
        m_tracks->clearTracks();
        AppConfig::setStatic("source", m_source->id());

        emit activeSourceChanged(m_source->id());
    }
}

void ApiController::onTrackUpdated(const QString& trackId, const QVariantMap& fields) {
    m_tracks->updateMetadata(trackId, fields);

    // A new song usually means new artwork; onActiveChanged() re-resolves it (and
    // does nothing when the URL is one we have already rendered).
    Track* active = m_tracks->getActive();
    if (active != NULL && active->getId().compare(trackId) == 0) {
        onActiveChanged(active);
    }
}

void ApiController::onTracksLoaded(const QVariantList& tracks) {
    QVariantList updated;
    QList<Track*> tracksList;

    foreach(QVariant var, tracks) {
        QVariantMap map = var.toMap();

        // A track already in favourites keeps its existing object: it carries the
        // downloaded file path and the favourite flag the list needs to draw the
        // heart. (The old code dropped these from `updated`, so a favourite track
        // silently went missing from the playlist.)
        Track* track = m_tracks->findFavouriteById(map.value("id").toString());
        if (track == NULL) {
            track = new Track(this);
            track->fromMap(map);
        }

        tracksList.append(track);
        updated.append(track->toMap());
    }

    m_tracks->appendTracks(tracksList);
    emit loaded(updated);
}

void ApiController::onLoadFailed(const QString& message) {
    qDebug() << "===>>> ApiController#onLoadFailed: " << message << endl;
    m_pToast->setBody(tr("Service is unavailable. Try later."));
    m_pToast->show();

    // Still answer: the list view keeps its spinner up until `loaded` arrives,
    // and the player waits for it before trying the next track.
    emit loaded(QVariantList());
}

void ApiController::onActiveChanged(Track* track) {
    // Tell the source what is playing: a radio source starts polling its station
    // for the current song, a catalogue ignores it.
    if (m_source != NULL) {
        m_source->setActiveTrack(track == NULL ? QString("") : track->getId());
    }

    if (track == NULL || track->getArtworkUrl().isEmpty()) {
        return;
    }

    // The rendered files are named after the artwork URL, so a cache hit costs a
    // stat rather than a download - and the original never has to stay around.
    QString coverPath = ArtworkProcessor::coverPathFor(track->getArtworkUrl());
    QString blurPath = ArtworkProcessor::blurPathFor(track->getArtworkUrl());
    if (QFile::exists(coverPath) && QFile::exists(blurPath)) {
        m_tracks->setImagePath(track->getId(), coverPath);
        m_tracks->setBlurImagePath(track->getId(), blurPath);
        return;
    }

    loadImage(track->getId(), track->getArtworkUrl());
}

void ApiController::onArtworkReady(const QString& trackId, const QString& coverPath, const QString& blurPath) {
    // The original is 2-5 MB and nothing reads it again once the cover and the
    // blur exist; keeping it would grow the cache by an order of magnitude.
    QString original = m_originals.take(trackId);
    if (!original.isEmpty()) {
        QFile::remove(original);
    }

    if (coverPath.isEmpty()) {
        return;   // render failed - the player keeps the default cassette art
    }

    m_tracks->setImagePath(trackId, coverPath);
    m_tracks->setBlurImagePath(trackId, blurPath);
}

void ApiController::loadImage(const QString& id, const QString& path) {
    if (m_artworkInFlight.contains(id)) {
        return;   // going back to a track whose cover is still downloading
    }
    m_artworkInFlight.insert(id);

    QString filename = path.split("/").last();
    QString filepath = QDir::currentPath() + IMAGES + "/" + filename;

    QFile file(filepath);
    if (file.exists()) {
        // A leftover original from a render that failed or was interrupted.
        m_artworkInFlight.remove(id);
        m_originals.insert(id, filepath);
        m_artwork->process(id, filepath, path);
        return;
    }

    QNetworkRequest req;
    req.setUrl(QUrl(path));

    QNetworkReply* reply = m_network->get(req);
    reply->setProperty("id", id);
    reply->setProperty("path", path);
    reply->setProperty("filepath", filepath);
    bool res = QObject::connect(reply, SIGNAL(finished()), this, SLOT(onImageLoad()));
    Q_ASSERT(res);
    res = QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onImageError(QNetworkReply::NetworkError)));
    Q_UNUSED(res);
}

void ApiController::onImageLoad() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    QString id = reply->property("id").toString();
    QString filepath = reply->property("filepath").toString();
    m_artworkInFlight.remove(id);

    QByteArray data = reply->readAll();

    if (data.size() != 0 && reply->error() == QNetworkReply::NoError) {
        QString imagesPath = QDir::currentPath() + IMAGES;
        QDir images(imagesPath);
        if (!images.exists()) {
            images.mkpath(imagesPath);
        }

        QFile image(filepath);
        if (image.open(QIODevice::WriteOnly)) {
            image.write(data);
            image.close();

            // Downscale + blur happen on a worker; onArtworkReady() publishes both.
            m_originals.insert(id, filepath);
            m_artwork->process(id, filepath, reply->property("path").toString());
        } else {
            qDebug() << "===>>> ApiController#onImageLoad cannot write " << filepath << " " << image.errorString() << endl;
        }
    }

    reply->deleteLater();
}

void ApiController::onImageError(QNetworkReply::NetworkError e) {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    m_artworkInFlight.remove(reply->property("id").toString());
    qDebug() << "===>>> ApiController#onImageError: " << e << " " << reply->errorString() << " " << reply->property("path").toString() << endl;
    reply->deleteLater();
}
