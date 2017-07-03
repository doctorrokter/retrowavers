/*
 * TracksService.cpp
 *
 *  Created on: Jun 24, 2017
 *      Author: misha
 */

#include "TracksService.hpp"
#include <QDir>
#include "../Common.hpp"
#include <QDebug>

TracksService::TracksService(QObject* parent) : QObject(parent), m_active(NULL) {}

TracksService::~TracksService() {
    m_active->deleteLater();
    foreach(Track* track, m_tracks) {
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
        m_tracks.append(track);
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
    }
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
}

void TracksService::setBlurImagePath(const QString& id, const QString& imagePath) {
    foreach(Track* track, m_tracks) {
        if (track->getId().compare(id) == 0) {
            track->setBImagePath("file://" + imagePath);
            emit imageChanged(id, imagePath);
        }
    }
}

QList<Track*>& TracksService::getTracksList() {
    return m_tracks;
}

QList<Track*>& TracksService::getFavouriteTracksList() {
    return m_favouriteTracks;
}
