/*
 * RetrowaveOneSource - retrowave.one, one station on a plain Icecast server.
 *
 * Dead on the genre and served over plain http, so mm-renderer never touches TLS
 * for it. status-json.xsl gives what is playing; there is no artwork anywhere in
 * that feed, so this station shows the default cassette.
 *
 * CAVEAT: the site's player reaches the stream by IP, and no hostname resolves to
 * it (stream./radio.retrowave.one do not exist). The address is therefore baked in
 * below - if the host ever moves, this station stops working and the constant has
 * to be updated. Nothing else in the app depends on it.
 */

#ifndef RETROWAVEONESOURCE_HPP_
#define RETROWAVEONESOURCE_HPP_

#include "ITrackSource.hpp"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVariantMap>

class QTimer;

class RetrowaveOneSource: public ITrackSource {
    Q_OBJECT
public:
    RetrowaveOneSource(QObject* parent = 0);
    virtual ~RetrowaveOneSource();

    QString id() const;
    QString name() const;
    QString kind() const;

    bool canList() const;
    bool canDownload() const;

    void loadMore();
    void reset();
    void setActiveTrack(const QString& trackId);

private slots:
    void onStatusReply();
    void onPollTimeout();

private:
    QNetworkAccessManager* m_network;
    QTimer* m_poll;
    bool m_delivered;
    QString m_activeTrackId;

    void requestNowPlaying();
};

#endif /* RETROWAVEONESOURCE_HPP_ */
