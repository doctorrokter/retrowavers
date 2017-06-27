/*
 * Track.cpp
 *
 *  Created on: Jun 26, 2017
 *      Author: misha
 */

#include "Track.hpp"

Track::Track(QObject* parent) : QObject(parent), m_id(""), m_duration(0), m_artworkUrl(""), m_bArtworkUrl(""), m_streamUrl(""), m_title(""), m_imagePath(""), m_bImagePath("") {}

Track::Track(const Track& track) : QObject(track.parent()) {
    swap(track);
}

Track::~Track() {}

bool Track::operator==(const Track& track) {
    return m_id.compare(track.getId()) == 0 && m_title.compare(track.getTitle()) == 0;
}

const QString& Track::getId() const { return m_id; }
void Track::setId(const QString& id) {
    if (id.compare(m_id) != 0) {
        m_id = id;
        emit idChanged(id);
    }
}

const int& Track::getDuration() const { return m_duration; }
void Track::setDuration(const int& duration) {
    if (m_duration != duration) {
        m_duration = duration;
        emit durationChanged(m_duration);
    }
}

const QString& Track::getStreamUrl() const { return m_streamUrl; }
void Track::setStreamUrl(const QString& streamUrl) {
    if (m_streamUrl.compare(streamUrl) != 0) {
        m_streamUrl = streamUrl;
        emit streamUrlChanged(m_streamUrl);
    }
}

const QString& Track::getArtworkUrl() const { return m_artworkUrl; }
void Track::setArtworkUrl(const QString& artworkUrl) {
    if (m_artworkUrl.compare(artworkUrl) != 0) {
        m_artworkUrl = artworkUrl;
        emit artworkUrlChanged(m_artworkUrl);
    }
}

const QString& Track::getBArtworkUrl() const { return m_bArtworkUrl; }
void Track::setBArtworkUrl(const QString& bArtworkUrl) {
    if (m_bArtworkUrl.compare(bArtworkUrl) != 0) {
        m_bArtworkUrl = bArtworkUrl;
        emit bArtworkUrlChanged(m_bArtworkUrl);
    }
}

const QString& Track::getTitle() const { return m_title; }
void Track::setTitle(const QString& title) {
    if (m_title.compare(title) != 0) {
        m_title = title;
        emit titleChanged(m_title);
    }
}

const QString& Track::getImagePath() const { return m_imagePath; }
void Track::setImagePath(const QString& imagePath) {
    if (m_imagePath.compare(imagePath) != 0) {
        m_imagePath = imagePath;
        emit imagePathChanged(m_imagePath);
    }
}

const QString& Track::getBImagePath() const { return m_bImagePath; }
void Track::setBImagePath(const QString& bImagePath) {
    if (m_bImagePath.compare(bImagePath) != 0) {
        m_bImagePath = bImagePath;
        emit bImagePathChanged(m_bImagePath);
    }
}

QVariantMap Track::toMap() {
    QVariantMap map;
    map["id"] = m_id;
    map["title"] = m_title;
    map["duration"] = m_duration;
    map["artworkUrl"] = m_artworkUrl;
    map["bArtworkUrl"] = m_bArtworkUrl;
    map["streamUrl"] = m_streamUrl;
    map["imagePath"] = m_imagePath;
    map["bImagePath"] = m_bImagePath;
    return map;
}

void Track::fromMap(const QVariantMap& map) {
    m_id = map.value("id").toString();
    m_title = map.value("title").toString();
    m_duration = map.value("duration").toInt();
    m_artworkUrl = map.value("artworkUrl").toString();
    m_bArtworkUrl = map.value("b_artworkUrl", "").toString();
    m_streamUrl = map.value("streamUrl").toString();
}

void Track::swap(const Track& track) {
    m_id = track.getId();
    m_duration = track.getDuration();
    m_title = track.getTitle();
    m_artworkUrl = track.getArtworkUrl();
    m_bArtworkUrl = track.getBArtworkUrl();
    m_streamUrl = track.getStreamUrl();
    m_imagePath = track.getImagePath();
    m_bImagePath = track.getBImagePath();
}
