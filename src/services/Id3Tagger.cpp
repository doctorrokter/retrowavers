/*
 * Id3Tagger.cpp - see Id3Tagger.hpp.
 */

#include "Id3Tagger.hpp"

#include <QFile>
#include <QDebug>

// A cover much bigger than this is not worth carrying inside every mp3 - the
// rendered cover is ~100 KB as JPEG, so this only trips if the PNG fallback ran.
static const int kMaxPictureBytes = 2 * 1024 * 1024;

// One ID3v2.3 frame: 4-char id, 4-byte size (plain big-endian in v2.3 - only the
// TAG header size is syncsafe), 2 flag bytes, then the body.
static QByteArray frame(const char* id, const QByteArray& body) {
    QByteArray out;
    if (body.isEmpty()) {
        return out;
    }

    const int size = body.size();
    out.append(id, 4);
    out.append(static_cast<char>((size >> 24) & 0xFF));
    out.append(static_cast<char>((size >> 16) & 0xFF));
    out.append(static_cast<char>((size >> 8) & 0xFF));
    out.append(static_cast<char>(size & 0xFF));
    out.append(static_cast<char>(0x00));
    out.append(static_cast<char>(0x00));
    out.append(body);
    return out;
}

// Text frames go out as UTF-16 with a byte-order mark (encoding byte 0x01).
// ISO-8859-1 would be shorter but silently mangles anything outside latin-1, and
// track titles are not ours to guess about.
static QByteArray textFrame(const char* id, const QString& value) {
    if (value.isEmpty()) {
        return QByteArray();
    }

    QByteArray body;
    body.append(static_cast<char>(0x01));
    body.append(static_cast<char>(0xFF));   // BOM, little-endian
    body.append(static_cast<char>(0xFE));
    for (int i = 0; i < value.size(); ++i) {
        const ushort code = value.at(i).unicode();
        body.append(static_cast<char>(code & 0xFF));
        body.append(static_cast<char>((code >> 8) & 0xFF));
    }
    return frame(id, body);
}

static QByteArray pictureFrame(const QString& coverPath) {
    if (coverPath.isEmpty()) {
        return QByteArray();
    }

    QFile file(coverPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "===>>> Id3Tagger: cannot read cover " << coverPath << endl;
        return QByteArray();
    }
    const QByteArray picture = file.readAll();
    file.close();

    if (picture.isEmpty() || picture.size() > kMaxPictureBytes) {
        qDebug() << "===>>> Id3Tagger: cover skipped, size " << picture.size() << endl;
        return QByteArray();
    }

    QByteArray body;
    body.append(static_cast<char>(0x00));   // ISO-8859-1: the description below is empty anyway
    body.append(coverPath.endsWith(".png", Qt::CaseInsensitive) ? "image/png" : "image/jpeg");
    body.append(static_cast<char>(0x00));   // MIME terminator
    body.append(static_cast<char>(0x03));   // picture type: cover (front)
    body.append(static_cast<char>(0x00));   // empty description
    body.append(picture);
    return frame("APIC", body);
}

QByteArray Id3Tagger::buildTag(const QString& title, const QString& artist, const QString& album,
                               const QString& genre, const QString& coverPath) {
    QByteArray frames;
    frames.append(textFrame("TIT2", title));
    frames.append(textFrame("TPE1", artist));
    frames.append(textFrame("TALB", album));
    frames.append(textFrame("TCON", genre));
    frames.append(pictureFrame(coverPath));

    if (frames.isEmpty()) {
        return QByteArray();
    }

    // The header size is "syncsafe": 28 bits spread over 4 bytes, 7 bits each, so
    // no byte can look like an mp3 sync word.
    const int size = frames.size();
    QByteArray tag;
    tag.append("ID3", 3);
    tag.append(static_cast<char>(0x03));   // version 2.3.0
    tag.append(static_cast<char>(0x00));
    tag.append(static_cast<char>(0x00));   // no unsynchronisation, no extended header
    tag.append(static_cast<char>((size >> 21) & 0x7F));
    tag.append(static_cast<char>((size >> 14) & 0x7F));
    tag.append(static_cast<char>((size >> 7) & 0x7F));
    tag.append(static_cast<char>(size & 0x7F));
    tag.append(frames);
    return tag;
}

QByteArray Id3Tagger::stripTag(const QByteArray& mp3) {
    if (mp3.size() < 10 || !mp3.startsWith("ID3")) {
        return mp3;
    }

    const quint8 flags = static_cast<quint8>(mp3.at(5));
    int size = 0;
    for (int i = 6; i < 10; ++i) {
        size = (size << 7) | (static_cast<quint8>(mp3.at(i)) & 0x7F);
    }

    int total = 10 + size;
    if (flags & 0x10) {
        total += 10;   // a footer is present (rare, but it is part of the tag)
    }

    if (total <= 0 || total >= mp3.size()) {
        qDebug() << "===>>> Id3Tagger: implausible tag size " << total << " in " << mp3.size() << " bytes" << endl;
        return mp3;
    }
    return mp3.mid(total);
}
