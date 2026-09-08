/*
 * RetrowaveRadioSource - the retrowave-radio.ru catalogue.
 *
 * Successor to the dead retrowave.ru. The API is documented in API.md; the two
 * things that shape this class:
 *
 *   1. There is no cursor. Paging is a `sessionId` the server remembers: within
 *      one session it never repeats a track until the whole catalogue (~94
 *      tracks) has been handed out, then it starts over. So we dedupe by id and
 *      an empty batch means "you have seen everything".
 *
 *   2. A track arrives as name + author, not one title string. We join them with
 *      an en dash, which is the separator the old API used and the one the
 *      player still splits on for the "now playing" overlay.
 */

#ifndef RETROWAVERADIOSOURCE_HPP_
#define RETROWAVERADIOSOURCE_HPP_

#include "ITrackSource.hpp"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSet>
#include <QVariantMap>

class RetrowaveRadioSource: public ITrackSource {
    Q_OBJECT
public:
    RetrowaveRadioSource(QObject* parent = 0);
    virtual ~RetrowaveRadioSource();

    QString id() const;
    QString name() const;
    QString kind() const;

    bool canList() const;
    bool canDownload() const;

    void loadMore();
    void reset();

private slots:
    void onTracksReply();

private:
    QNetworkAccessManager* m_network;
    QString m_sessionId;
    QSet<QString> m_seen;
    bool m_inFlight;

    QVariantMap toTrackMap(const QVariantMap& json) const;
};

#endif /* RETROWAVERADIOSOURCE_HPP_ */
