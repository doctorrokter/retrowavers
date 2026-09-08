/*
 * ArtworkProcessor - turns a downloaded cover into the two images the UI needs.
 *
 * The new API serves 1254x1254 (sometimes 2048x2048) PNGs of 2-5 MB. Handing one
 * of those straight to an ImageView costs memory and decode time on every layout,
 * so each cover is rendered once into:
 *
 *   cover - downscaled artwork for the cassette and the "now playing" icon
 *   blur  - the soft backdrop behind the player
 *
 * The blur used to come from a web service (retrowavers.herokuapp.com/blur/65/...)
 * because there was no blur on the device. There is now: StackBlur, ported from
 * BBTelega's RoundedImageProvider. No round trip, works offline, and one less
 * dependency that can die like the old API did.
 *
 * Rendering happens on a worker thread (QtConcurrent) - a 2 MB PNG decode on the
 * UI thread is a visible stall. Output paths are deterministic, so a caller can
 * check the cache before downloading anything.
 */

#ifndef ARTWORKPROCESSOR_HPP_
#define ARTWORKPROCESSOR_HPP_

#include <QObject>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QSet>
#include <QFutureWatcher>

class ArtworkProcessor: public QObject {
    Q_OBJECT
public:
    ArtworkProcessor(QObject* parent = 0);
    virtual ~ArtworkProcessor();

    // Where the rendered files for this artwork URL live (they may not exist yet).
    static QString coverPathFor(const QString& artworkUrl);
    static QString blurPathFor(const QString& artworkUrl);

    // Render both images from sourcePath. Answers with ready(); on failure both
    // paths come back empty, so the caller can fall back to the default cover.
    // De-duplicated by source path - asking twice while one render is in flight
    // is a no-op, and every listener gets the result.
    void process(const QString& trackId, const QString& sourcePath, const QString& artworkUrl);

    Q_SIGNALS:
        void ready(const QString& trackId, const QString& coverPath, const QString& blurPath);

private slots:
    void onFinished();

private:
    QHash<QFutureWatcher<QStringList>*, QString> m_pendingTrack;
    QHash<QFutureWatcher<QStringList>*, QString> m_pendingSource;
    QSet<QString> m_inFlight;

    // Worker-thread entry point: touches no members, so it is safe off the UI thread.
    static QStringList render(const QString& sourcePath, const QString& coverPath, const QString& blurPath);
};

#endif /* ARTWORKPROCESSOR_HPP_ */
