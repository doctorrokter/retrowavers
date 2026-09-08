/*
 * MusicLibrary - where a downloaded track ends up.
 *
 * Liked tracks used to land in the app sandbox (data/tracks), invisible to
 * everything else on the device. They now go into the device MUSIC folder, under
 * a Retrowavers subdirectory, so the system music player indexes them like any
 * other album - which is also why the ID3 tag is rebuilt on the way in
 * (see Id3Tagger).
 *
 * The location is resolved the same way the other apps here do it (BBTube's
 * DownloadManager): SD card first, then the device shared filesystem, and only
 * if neither is writable the sandbox - where the file still plays but stays
 * invisible to the music player and dies with an uninstall.
 */

#ifndef MUSICLIBRARY_HPP_
#define MUSICLIBRARY_HPP_

#include <QObject>
#include <QString>
#include <QByteArray>

#include "../models/Track.hpp"

class MusicLibrary: public QObject {
    Q_OBJECT
public:
    MusicLibrary(QObject* parent = 0);
    virtual ~MusicLibrary();

    // Resolved on first use and cached; refresh() re-resolves it (a card can be
    // inserted or pulled while the app runs).
    QString baseDir();
    QString storageLabel();
    void refresh();

    // Write the track with a rebuilt ID3 tag (cover embedded when coverPath is a
    // readable image). Returns the file path, or "" if nothing could be written.
    QString save(Track* track, const QByteArray& mp3, const QString& coverPath);

private:
    QString m_baseDir;
    QString m_label;

    void resolve();
    static bool dirWritable(const QString& dir);
    static QString fileNameFor(Track* track);
};

#endif /* MUSICLIBRARY_HPP_ */
