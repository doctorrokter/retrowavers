/*
 * TracksController.cpp
 *
 *  Created on: Jun 24, 2017
 *      Author: misha
 */

#include "TracksController.hpp"

#define NOTIFICATION_KEY "Retrowavers"

TracksController::TracksController(TracksService* tracks, QObject* parent) : QObject(parent), m_tracks(tracks), m_index(0) {
    m_pNotification = new Notification(this);
    m_pNotification->setTitle(NOTIFICATION_KEY);
    m_pNotification->setType(NotificationType::AllAlertsOff);
    m_pNotification->deleteAllFromInbox();
}

TracksController::~TracksController() {
    m_pNotification->deleteAllFromInbox();
    m_pNotification->deleteLater();
}

void TracksController::play(const QVariantMap& track) {
    m_index = 0;
    if (track.contains("id")) {
        for (int i = 0; i < m_tracks->count(); i++) {
            QString id = track.value("id").toString();
            if (m_tracks->getTracksList().at(i)->getId().compare(id) == 0) {
                m_index = i;

                Track* pTrack = m_tracks->findById(id);
                m_tracks->setActive(pTrack);
                notify(pTrack);
                emit played(pTrack->toMap());
            }
        }
    } else {
        Track* pTrack = m_tracks->getTracksList().at(m_index);
        m_tracks->setActive(pTrack);
        notify(pTrack);
        emit played(pTrack->toMap());
    }
}

bool TracksController::next() {
    if (m_index < (m_tracks->count() -1)) {
        m_index++;
        play(m_tracks->getTracks().at(m_index).toMap());
        return true;
    }
    return false;
}

bool TracksController::prev() {
    if (m_index != 0) {
        m_index--;
        play(m_tracks->getTracks().at(m_index).toMap());
        return true;
    }
    return false;
}

void TracksController::notify(Track* track) {
    m_pNotification->setBody(track->getTitle());
    m_pNotification->notify();
}
