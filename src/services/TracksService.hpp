/*
 * TracksService.hpp
 *
 *  Created on: Jun 24, 2017
 *      Author: misha
 */

#ifndef TRACKSSERVICE_HPP_
#define TRACKSSERVICE_HPP_

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QList>
#include "../models/Track.hpp"

class TracksService: public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList tracks READ getTracks WRITE setTracks NOTIFY tracksChanged)
    Q_PROPERTY(QVariantList favouriteTracks READ getFavouriteTracks NOTIFY favouriteTracksChanged)
    Q_PROPERTY(Track* active READ getActive NOTIFY activeChanged)
public:
    TracksService(QObject* parent = 0);
    virtual ~TracksService();

    Q_INVOKABLE QVariantList getTracks() const;
    Q_INVOKABLE void setTracks(const QVariantList& tracks);
    Q_INVOKABLE Track* findById(const QString& id);
    Q_INVOKABLE Track* findFavouriteById(const QString& id);
    Q_INVOKABLE Track* getActive() const;

    Q_INVOKABLE QVariantList getFavouriteTracks() const;

    void setActive(Track* track);
    int count() const;
    void appendTracks(const QList<Track*>& tracks);
    void addFavourite(Track* track);
    bool removeFavourite(const QString& id);
    void setImagePath(const QString& id, const QString& imagePath);
    void setBlurImagePath(const QString& id, const QString& imagePath);
    QList<Track*>& getTracksList();
    QList<Track*>& getFavouriteTracksList();

    Q_SIGNALS:
        void tracksChanged(const QVariantList& tracks);
        void favouriteTracksChanged(const QVariantList& favouriteTracks);
        void activeChanged(Track* track);
        void imageChanged(const QString& id, const QString& imagePath);
        void blurImageChanged(const QString& id, const QString& imagePath);

private:
    QList<Track*> m_tracks;
    QList<Track*> m_favouriteTracks;
    Track* m_active;

    void saveFavourite();
};

#endif /* TRACKSSERVICE_HPP_ */
