/*
 * TracksService.cpp
 *
 *  Created on: Jun 24, 2017
 *      Author: misha
 */

#include "TracksService.hpp"
#include <QDir>
#include <QFile>
#include "../Common.hpp"
#include <QDebug>
#include <QVariant>
#include <QVariantList>
#include <bb/data/JsonDataAccess>

using namespace bb::data;

TracksService::TracksService(QObject* parent) : QObject(parent), m_active(NULL) {
    QFile file(QDir::currentPath() + FAVORITE_TRACKS);
    if (file.exists()) {
        JsonDataAccess jda;
        QVariantList list = jda.load(&file).toList();
        foreach(QVariant var, list) {
            Track* track = new Track();
            track->fromMap(var.toMap());
            m_favouriteTracks.append(track);
            qDebug() << track->toMap() << endl;
        }
        qDebug() << "Favourite tracks: " << m_favouriteTracks.size() << endl;
    }
}

TracksService::~TracksService() {
    saveFavourites();
    m_active->deleteLater();
    foreach(Track* track, m_tracks) {
        track->deleteLater();
    }
    foreach(Track* track, m_favouriteTracks) {
        track->deleteLater();
    }
}

QVariantList TracksService::getTracks() const {
    QVariantList tracks;
    foreach(Track* track, m_tracks) {
        tracks.append(track->toMap());
    }
    return tracks;
}

void TracksService::setTracks(const QVariantList& tracks) {
    foreach(QVariant var, tracks) {
        Track* track = new Track();
        track->fromMap(var.toMap());
        m_tracks.append(track);
    }
    emit tracksChanged(tracks);
}

void TracksService::appendTracks(const QList<Track*>& tracks) {
    m_tracks.append(tracks);
    QVariantList tracksMaps;
    foreach(Track* track, tracks) {
        tracksMaps.append(track->toMap());
    }
    emit tracksChanged(tracksMaps);
}

void TracksService::addFavourite(Track* track) {
    bool exists = false;
    foreach(Track* t, m_favouriteTracks) {
        exists = t == track;
    }

    if (!exists) {
        m_favouriteTracks.append(track);
        emit favouriteTracksChanged(getFavouriteTracks());
        saveFavourites();
    }
}

bool TracksService::removeFavourite(const QString& id) {
    Track* track = findFavouriteById(id);
    if (track != NULL) {
        if (m_favouriteTracks.removeOne(track)) {
            QFile file(track->getLocalPath());
            if (file.exists()) {
                qDebug() << " Removing file: " << track->getLocalPath() << endl;
                file.remove();
            }

            if (!m_tracks.contains(track)) {
                track->deleteLater();
            }

            saveFavourites();
            return true;
        }
        return false;
    }
    return false;
}

Track* TracksService::findById(const QString& id) {
    for (int i = 0; i < m_tracks.size(); i++) {
        Track* track = m_tracks.at(i);
        if (track->getId().compare(id) == 0) {
            return track;
        }
    }
    return NULL;
}

Track* TracksService::findFavouriteById(const QString& id) {
    for (int i = 0; i < m_favouriteTracks.size(); i++) {
        Track* track = m_favouriteTracks.at(i);
        if (track->getId().compare(id) == 0) {
            return track;
        }
    }
    return NULL;
}

Track* TracksService::getActive() const {
    return m_active;
}

void TracksService::setActive(Track* track) {
    if (m_active != track) {
        m_active = track;
        emit activeChanged(m_active);
    }
}

QVariantList TracksService::getFavouriteTracks() const {
    QVariantList tracks;
    foreach(Track* track, m_favouriteTracks) {
        tracks.append(track->toMap());
    }
    return tracks;
}

void TracksService::clearTracks() {
    foreach(Track* track, m_tracks) {
        if (track != m_active && !m_favouriteTracks.contains(track)) {
            track->deleteLater();
        }
    }
    m_tracks.clear();
    emit tracksChanged(QVariantList());
}

int TracksService::migrateLegacyFavourites() {
    int migrated = 0;

    foreach(Track* track, m_favouriteTracks) {
        if (track->getId().contains(":")) {
            continue;   // already namespaced by a source (rwr:, soma:, ...)
        }

        // Old ids were bare numbers - and so are retrowave-radio.ru's, so without a
        // namespace a legacy favourite could shadow a brand new track.
        track->setId("legacy:" + track->getId());

        // Both hosts are gone: retrowave.ru itself and the blur service it used.
        // Clearing them stops the app from ever trying, and the blur is rendered on
        // the device now anyway.
        track->setBArtworkUrl("");
        track->setBImagePath("");
        if (track->getArtworkUrl().contains("retrowave.ru")) {
            track->setArtworkUrl("");
        }
        if (track->getStreamUrl().contains("retrowave.ru")) {
            track->setStreamUrl("");   // only the downloaded file can still play it
        }

        migrated++;
    }

    if (migrated > 0) {
        saveFavourites();
    }
    return migrated;
}

void TracksService::updateMetadata(const QString& id, const QVariantMap& fields) {
    Track* track = findById(id);
    if (track == NULL) {
        track = findFavouriteById(id);
    }
    if (track == NULL) {
        return;
    }

    if (fields.contains("title")) {
        track->setTitle(fields.value("title").toString());
    }
    if (fields.contains("author")) {
        track->setAuthor(fields.value("author").toString());
    }
    if (fields.contains("name")) {
        track->setName(fields.value("name").toString());
    }
    if (fields.contains("artworkUrl")) {
        track->setArtworkUrl(fields.value("artworkUrl").toString());
        track->setImagePath("");   // the old cover belongs to the previous song
    }

    emit metadataChanged(id, track->getTitle());
}

int TracksService::count() const {
    return m_tracks.size();
}

void TracksService::setImagePath(const QString& id, const QString& imagePath) {
    foreach(Track* track, m_tracks) {
        if (track->getId().compare(id) == 0) {
            track->setImagePath("file://" + imagePath);
            emit imageChanged(id, imagePath);
        }
    }

    foreach(Track* track, m_favouriteTracks) {
        if (track->getId().compare(id) == 0 && !m_tracks.contains(track)) {
            track->setImagePath("file://" + imagePath);
            emit imageChanged(id, imagePath);
        }
    }
}

void TracksService::setBlurImagePath(const QString& id, const QString& imagePath) {
    foreach(Track* track, m_tracks) {
        if (track->getId().compare(id) == 0) {
            track->setBImagePath("file://" + imagePath);
            emit blurImageChanged(id, track->getBImagePath());
        }
    }

    foreach(Track* track, m_favouriteTracks) {
        if (track->getId().compare(id) == 0) {
            track->setBImagePath("file://" + imagePath);
            emit blurImageChanged(id, track->getBImagePath());
        }
    }
}

QList<Track*>& TracksService::getTracksList() {
    return m_tracks;
}

QList<Track*>& TracksService::getFavouriteTracksList() {
    return m_favouriteTracks;
}

void TracksService::saveFavourites() {
    QVariantList list;
    foreach(Track* track, m_favouriteTracks) {
        list.append(track->toMap());
    }

    JsonDataAccess jda;
    QFile file(QDir::currentPath() + FAVORITE_TRACKS);
    if (file.open(QIODevice::WriteOnly)) {
        jda.save(QVariant(list), &file);
        qDebug() << "Save favourite tracks: " << QDir::currentPath() + FAVORITE_TRACKS << endl;
    }
}
