/*
 * Id3Tagger - builds the ID3v2.3 tag we write into a downloaded track.
 *
 * The files served by the API already carry tags, but they cannot be trusted:
 * of three tracks sampled, one had TPE1 "retrowave-radio.ru", one had no title
 * and no artist at all, and a third named an artist the API does not agree with.
 * All of them carry a "made with suno" comment and a link frame. Copying such a
 * file into the device music library gives rows with no name and a domain as the
 * performer, so the tag is REBUILT from what the API told us instead.
 *
 * Only the frames the BB10 music player actually reads are written: title,
 * artist, album, genre and the cover picture.
 */

#ifndef ID3TAGGER_HPP_
#define ID3TAGGER_HPP_

#include <QByteArray>
#include <QString>

class Id3Tagger {
public:
    // A complete ID3v2.3 tag (header + frames), ready to sit in front of the
    // audio frames. Empty fields are skipped; coverPath ("" for none) is embedded
    // as an APIC picture, its MIME type taken from the extension.
    static QByteArray buildTag(const QString& title, const QString& artist, const QString& album,
                               const QString& genre, const QString& coverPath);

    // mp3 bytes with any leading ID3v2 tag removed. Returns the input unchanged
    // when there is no tag (an ID3v1 tag, if present, sits at the END and is
    // harmless, so it is left alone).
    static QByteArray stripTag(const QByteArray& mp3);
};

#endif /* ID3TAGGER_HPP_ */
