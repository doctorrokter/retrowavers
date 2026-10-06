/*
 * RadioSource - the shared half of every radio service.
 *
 * Six services turned out to differ in only two respects: where the station list
 * comes from, and how "what is on air" is parsed. Everything else - the poll timer,
 * the network manager, turning a station into the track map the model expects,
 * ignoring replies for a station that is no longer playing - was copied six times.
 * It lives here now, and a service is what is left over:
 *
 *   requestStations()   fill the list (synchronously or via a request) and hand it
 *                       to deliverStations()
 *   nowPlayingUrl()     where to ask what is playing ("" if the service cannot say)
 *   parseNowPlaying()   turn that reply into title/author/name (+ artworkUrl)
 *
 * A service whose metadata does not arrive as one plain GET (Nightride's endless
 * event stream) overrides requestNowPlaying() instead.
 *
 * Polling runs ONLY while one of this service's stations is playing: setActiveTrack()
 * starts and stops the timer, so a service nobody is listening to costs nothing.
 */

#ifndef RADIOSOURCE_HPP_
#define RADIOSOURCE_HPP_

#include "ITrackSource.hpp"

#include <QList>
#include <QString>
#include <QByteArray>
#include <QVariantMap>

class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

class RadioSource: public ITrackSource {
    Q_OBJECT
public:
    RadioSource(QObject* parent = 0);
    virtual ~RadioSource();

    // Same for every radio service: a browsable list of live stations, and nothing
    // to download - there is no file behind a stream.
    QString kind() const;
    bool canList() const;
    bool canDownload() const;

    void loadMore();
    void reset();
    void setActiveTrack(const QString& trackId);

protected:
    // One station as a service describes it. `key` is both the suffix of the track
    // id ("<source>:<key>") and whatever the service calls this station.
    struct Station {
        QString key;
        QString title;
        QString genre;
        QString streamUrl;
        QString artworkUrl;   // usually empty: most services have no station art
    };

    // Called once, when the list is first needed. Either build it and pass it to
    // deliverStations() right away, or start a request and call deliverStations()
    // when it answers.
    virtual void requestStations() = 0;

    // Where to ask what is on air. "" means this service cannot tell us.
    virtual QString nowPlayingUrl(const QString& stationKey) const;

    // Turn a metadata reply into fields for trackUpdated(): title/author/name, and
    // artworkUrl when the service has per-track art. false = nothing usable.
    virtual bool parseNowPlaying(const QByteArray& body, const QString& stationKey,
                                 QVariantMap& fields) const;

    // Default: a plain GET of nowPlayingUrl() parsed by parseNowPlaying(). Override
    // for anything that is not one request with one body.
    virtual void requestNowPlaying(const QString& stationKey);

    // Answer requestStations(). Emits tracksLoaded() with the stations as tracks.
    void deliverStations(const QList<Station>& stations);

    // "<source>:<key>" -> "key", or "" when the id belongs to another source.
    QString stationKeyOf(const QString& trackId) const;

    QString activeTrackId() const;
    QNetworkAccessManager* network() const;

    // "Artist - Title" the way the rest of the app expects it (en dash), or just the
    // title when there is no artist.
    static QString joinArtistTitle(const QString& artist, const QString& title);

    // Publish parsed metadata for the station currently playing.
    void publishNowPlaying(const QString& trackId, const QVariantMap& fields);

private slots:
    void onNowPlayingReply();
    void onPollTimeout();

private:
    QNetworkAccessManager* m_network;
    QTimer* m_poll;
    QList<Station> m_stations;
    bool m_delivered;
    QString m_activeTrackId;

    QVariantMap toTrackMap(const Station& station) const;
};

#endif /* RADIOSOURCE_HPP_ */
