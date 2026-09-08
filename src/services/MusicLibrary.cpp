/*
 * MusicLibrary.cpp - see MusicLibrary.hpp.
 */

#include "MusicLibrary.hpp"
#include "Id3Tagger.hpp"
#include "../Common.hpp"

#include <QDir>
#include <QFile>
#include <QStringList>
#include <QDebug>

// Standard BB10 mount points: the card is always here on 10.3.x, and the shared
// filesystem (granted by the access_shared permission) sits under .../shared.
static const char* kSdCardRoot = "/accounts/1000/removable/sdcard";
static const char* kSdCardMusic = "/accounts/1000/removable/sdcard/music";
static const char* kSharedMusic = "/accounts/1000/shared/music";
static const char* kAppSubdir = "Retrowavers";

static const char* kAlbum = "Retrowavers";

MusicLibrary::MusicLibrary(QObject* parent) : QObject(parent) {}

MusicLibrary::~MusicLibrary() {}

QString MusicLibrary::baseDir() {
    if (m_baseDir.isEmpty()) {
        resolve();
    }
    return m_baseDir;
}

QString MusicLibrary::storageLabel() {
    if (m_baseDir.isEmpty()) {
        resolve();
    }
    return m_label;
}

void MusicLibrary::refresh() {
    m_baseDir = "";
    resolve();
}

bool MusicLibrary::dirWritable(const QString& dir) {
    if (!QDir().mkpath(dir)) {
        return false;
    }

    // mkpath can succeed on a read-only mount, so prove it with a real write.
    QFile probe(dir + "/.rw_probe");
    if (!probe.open(QIODevice::WriteOnly)) {
        return false;
    }
    probe.write("1");
    probe.close();
    QFile::remove(dir + "/.rw_probe");
    return true;
}

void MusicLibrary::resolve() {
    // No SdCardInfo here: its state() says a card is mounted, not that we may write
    // to it - a read-only card passes the check and fails the write anyway. The
    // mount point plus a real write probe answers the only question that matters.
    if (QDir(kSdCardRoot).exists()) {
        const QString dir = QString(kSdCardMusic) + "/" + kAppSubdir;
        if (dirWritable(dir)) {
            m_baseDir = dir;
            m_label = "SD card";
            qDebug() << "===>>> MusicLibrary: " << m_baseDir << " (" << m_label << ")" << endl;
            return;
        }
    }

    const QString shared = QString(kSharedMusic) + "/" + kAppSubdir;
    if (dirWritable(shared)) {
        m_baseDir = shared;
        m_label = "Device storage";
        qDebug() << "===>>> MusicLibrary: " << m_baseDir << " (" << m_label << ")" << endl;
        return;
    }

    // Last resort: plays fine, but the music player never sees it and an
    // uninstall takes it with the app.
    m_baseDir = QDir::currentPath() + TRACKS;
    m_label = "App storage";
    QDir().mkpath(m_baseDir);
    qDebug() << "===>>> MusicLibrary: " << m_baseDir << " (" << m_label << ")" << endl;
}

QString MusicLibrary::fileNameFor(Track* track) {
    QString base;
    if (!track->getAuthor().isEmpty() && !track->getName().isEmpty()) {
        base = track->getAuthor() + " - " + track->getName();
    } else if (!track->getTitle().isEmpty()) {
        base = track->getTitle();
    } else {
        base = track->getFilename();
    }

    // Characters no filesystem here is happy about, plus control codes.
    QString safe;
    for (int i = 0; i < base.size(); ++i) {
        const QChar c = base.at(i);
        if (c < QChar(0x20) || QString("/\\:*?\"<>|").contains(c)) {
            safe.append("_");
        } else {
            safe.append(c);
        }
    }

    safe = safe.trimmed();
    if (safe.isEmpty()) {
        safe = "track";
    }
    if (safe.size() > 100) {
        safe = safe.left(100).trimmed();
    }
    if (safe.endsWith(".mp3", Qt::CaseInsensitive)) {
        return safe;
    }
    return safe + ".mp3";
}

QString MusicLibrary::save(Track* track, const QByteArray& mp3, const QString& coverPath) {
    if (track == NULL || mp3.isEmpty()) {
        return QString("");
    }

    const QString dir = baseDir();
    if (!QDir().mkpath(dir)) {
        qDebug() << "===>>> MusicLibrary: cannot create " << dir << endl;
        return QString("");
    }

    const QString genre = track->getTags().isEmpty() ? QString("") : track->getTags().first();
    const QByteArray tag = Id3Tagger::buildTag(track->getName().isEmpty() ? track->getTitle() : track->getName(),
                                               track->getAuthor(), kAlbum, genre, coverPath);
    const QByteArray audio = Id3Tagger::stripTag(mp3);

    const QString path = dir + "/" + fileNameFor(track);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "===>>> MusicLibrary: cannot write " << path << " " << file.errorString() << endl;
        return QString("");
    }
    file.write(tag);
    file.write(audio);
    file.close();

    qDebug() << "===>>> MusicLibrary: saved " << path << " tag " << tag.size() << " audio " << audio.size() << endl;
    return path;
}
