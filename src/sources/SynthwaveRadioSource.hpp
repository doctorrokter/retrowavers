/*
 * SynthwaveRadioSource - synthwaveradio.eu, a single station running AzuraCast.
 *
 * Worth having for two reasons: the genre is exactly this app's, and AzuraCast's
 * /api/nowplaying reports the ARTWORK of the current track - so, like Nightwave
 * Plaza, this station gets a real cassette and a real blurred backdrop instead of
 * the default cover.
 *
 * Its stream is https only (plain http answers 301). That is fine here: the
 * device already plays the https catalogue, so mm-renderer copes - but it is the
 * reason this source cannot claim the "http sidesteps TLS" advantage the other
 * radio services have.
 */

#ifndef SYNTHWAVERADIOSOURCE_HPP_
#define SYNTHWAVERADIOSOURCE_HPP_

#include "ITrackSource.hpp"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVariantMap>

class QTimer;

class SynthwaveRadioSource: public ITrackSource {
    Q_OBJECT
public:
    SynthwaveRadioSource(QObject* parent = 0);
    virtual ~SynthwaveRadioSource();

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

#endif /* SYNTHWAVERADIOSOURCE_HPP_ */
