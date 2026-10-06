/*
 * IcecastSource.cpp - see IcecastSource.hpp.
 */

#include "IcecastSource.hpp"

#include <bb/data/JsonDataAccess>

#include <QVariantList>

using namespace bb::data;

IcecastSource::IcecastSource(QObject* parent) : RadioSource(parent) {}

QString IcecastSource::nowPlayingUrl(const QString& stationKey) const {
    Q_UNUSED(stationKey);
    return statusUrl();   // one document covers every mount
}

bool IcecastSource::parseNowPlaying(const QByteArray& body, const QString& stationKey,
                                    QVariantMap& fields) const {
    JsonDataAccess jda;
    QVariantMap stats = jda.loadFromBuffer(body).toMap().value("icestats").toMap();

    // A single mount comes back as an object, several as a list.
    QVariantList sources = stats.value("source").toList();
    if (sources.isEmpty() && stats.contains("source")) {
        sources.append(stats.value("source"));
    }

    foreach(QVariant var, sources) {
        QVariantMap source = var.toMap();
        if (!source.value("listenurl").toString().endsWith("/" + stationKey)) {
            continue;
        }

        // Icecast packs it all into one field as "Artist - Title".
        const QString nowPlaying = source.value("title").toString().trimmed();
        if (nowPlaying.isEmpty()) {
            return false;
        }

        QString artist;
        QString song = nowPlaying;
        const int sep = nowPlaying.indexOf(" - ");
        if (sep > 0) {
            artist = nowPlaying.left(sep).trimmed();
            song = nowPlaying.mid(sep + 3).trimmed();
        }

        fields["title"] = joinArtistTitle(artist, song);
        fields["author"] = artist;
        fields["name"] = song;
        return true;
    }

    return false;
}
