/*
 * ApiController - the facade QML talks to.
 *
 *  Created on: Jun 24, 2017
 *      Author: misha
 *
 * It used to BE the retrowave.ru client. It now owns the track sources and
 * delegates to whichever is active, so the QML contract is unchanged: load() and
 * the `loaded` signal mean the same as they always did, and the maps carried by
 * that signal have the same shape.
 *
 * It also owns artwork. Fetching is LAZY - only the track being played, never a
 * whole page - because the new API serves 1254x1254 (sometimes 2048x2048) PNGs
 * of 2-5 MB each: pre-fetching a page of 25 would cost tens of megabytes for
 * images the list does not even show.
 */

#ifndef APICONTROLLER_HPP_
#define APICONTROLLER_HPP_

#include <QtCore/QObject>
#include <QVariantList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QList>
#include <QSet>
#include <QHash>
#include <bb/system/SystemToast>

#include "../services/TracksService.hpp"
#include "../services/ArtworkProcessor.hpp"
#include "../sources/ITrackSource.hpp"

using namespace bb::system;

class ApiController: public QObject {
    Q_OBJECT
public:
    ApiController(TracksService* tracks, QObject* parent = 0);
    virtual ~ApiController();

    Q_INVOKABLE void load();

    // The source registry. Nothing in QML uses these yet - they are the seam a
    // source picker will plug into, and they keep the single-source case honest.
    Q_INVOKABLE QVariantList sources() const;
    Q_INVOKABLE QString activeSource() const;

    // "catalog" or "radio" - the Radio tab needs to know whether the current list
    // is stations (and so wants a way back to the service list) or tracks.
    Q_INVOKABLE QString activeKind() const;
    Q_INVOKABLE void selectSource(const QString& sourceId);

    // Whether the ACTIVE source has files behind its tracks. Radio does not, so
    // QML hides the like/download button rather than offering a dead control.
    Q_INVOKABLE bool canDownload() const;

    void loadImage(const QString& id, const QString& path);

    Q_SIGNALS:
        void loaded(const QVariantList& songs);
        void activeSourceChanged(const QString& sourceId);

private slots:
    void onTracksLoaded(const QVariantList& tracks);
    void onLoadFailed(const QString& message);
    void onActiveChanged(Track* track);
    void onTrackUpdated(const QString& trackId, const QVariantMap& fields);
    void onArtworkReady(const QString& trackId, const QString& coverPath, const QString& blurPath);
    void onImageLoad();
    void onImageError(QNetworkReply::NetworkError e);

private:
    QNetworkAccessManager* m_network;
    TracksService* m_tracks;
    ArtworkProcessor* m_artwork;
    SystemToast* m_pToast;

    QList<ITrackSource*> m_sources;
    ITrackSource* m_source;

    QSet<QString> m_artworkInFlight;      // track ids whose cover is downloading
    QHash<QString, QString> m_originals;  // track id -> downloaded original, deleted once rendered

    void connectSource(ITrackSource* source);
};

#endif /* APICONTROLLER_HPP_ */
