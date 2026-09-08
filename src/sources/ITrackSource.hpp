/*
 * ITrackSource - where tracks come from.
 *
 * The app used to talk to one host directly from ApiController. It now talks to
 * whatever source is active, and ApiController is only a facade: it owns the
 * sources, delegates load() to the active one and keeps the QML-facing contract
 * (the `loaded` signal, the track map shape) unchanged.
 *
 * A source hands back tracks as QVariantMaps in the shape Track::fromMap()
 * expects, so the model and every QML consumer stay source-agnostic. That is
 * what makes a second source (a live radio stream, say) a matter of adding a
 * class rather than touching the UI: a station is simply a "track" whose
 * metadata changes while it plays.
 */

#ifndef ITRACKSOURCE_HPP_
#define ITRACKSOURCE_HPP_

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class ITrackSource: public QObject {
    Q_OBJECT
public:
    ITrackSource(QObject* parent = 0) : QObject(parent) {}
    virtual ~ITrackSource() {}

    // Stable, short identity. Doubles as the prefix of every track id this
    // source emits ("rwr:49"), which is what keeps ids from two sources - and
    // from the old retrowave.ru favourites - out of each other's way.
    virtual QString id() const = 0;
    virtual QString name() const = 0;

    // "catalog" - a library of tracks (the Playlist tab).
    // "radio"   - a service whose entries are live stations (the Radio tab lists
    //             the services, and picking one shows ITS stations - stations from
    //             different services are never mixed into one list).
    virtual QString kind() const = 0;

    // What the UI may offer. A radio stream has no browsable list and nothing
    // to download; a catalogue has both.
    virtual bool canList() const = 0;
    virtual bool canDownload() const = 0;

    // Ask for the next batch. Answers with tracksLoaded() - an EMPTY list is a
    // valid answer meaning "nothing new" (catalogue exhausted), and callers must
    // handle it: the list view keeps its spinner running until something lands.
    virtual void loadMore() = 0;

    // Forget pagination and start over.
    virtual void reset() = 0;

    // Which of our tracks is playing now ("" for none). A catalogue ignores this;
    // a radio source uses it to poll the station for what is on air and answer with
    // trackUpdated(). Not pure: most sources have nothing to do here.
    virtual void setActiveTrack(const QString& trackId) { Q_UNUSED(trackId); }

    Q_SIGNALS:
        void tracksLoaded(const QVariantList& tracks);

        // Fields of an already-delivered track changed underneath us - a radio
        // station whose current song moved on. Only the keys present are updated.
        void trackUpdated(const QString& trackId, const QVariantMap& fields);
        void loadFailed(const QString& message);
};

#endif /* ITRACKSOURCE_HPP_ */
