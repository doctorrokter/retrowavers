/*
 * TracksController.hpp
 *
 *  Created on: Jun 24, 2017
 *      Author: misha
 */

#ifndef TRACKSCONTROLLER_HPP_
#define TRACKSCONTROLLER_HPP_

#include <QObject>
#include <QVariantMap>
#include "../services/TracksService.hpp"

class TracksController: public QObject {
    Q_OBJECT
public:
    TracksController(TracksService* tracks, QObject* parent = 0);
    virtual ~TracksController();

    Q_INVOKABLE void play(const QVariantMap& track);
    Q_INVOKABLE void pause();
    Q_INVOKABLE bool next();
    Q_INVOKABLE bool prev();

    Q_SIGNALS:
        void played(const QVariantMap& track);
        void paused();

private:
    TracksService* m_tracks;
    int m_index;
};

#endif /* TRACKSCONTROLLER_HPP_ */
