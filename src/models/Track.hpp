/*
 * Track.hpp
 *
 *  Created on: Jun 26, 2017
 *      Author: misha
 */

#ifndef TRACK_HPP_
#define TRACK_HPP_

#include <QObject>
#include <QVariantMap>

class Track: public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ getId WRITE setId NOTIFY idChanged)
    Q_PROPERTY(int duration READ getDuration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(QString artworkUrl READ getArtworkUrl WRITE setArtworkUrl NOTIFY artworkUrlChanged)
    Q_PROPERTY(QString bArtworkUrl READ getBArtworkUrl WRITE setBArtworkUrl NOTIFY bArtworkUrlChanged)
    Q_PROPERTY(QString streamUrl READ getStreamUrl WRITE setStreamUrl NOTIFY streamUrlChanged)
    Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString imagePath READ getImagePath WRITE setImagePath NOTIFY imagePathChanged)
    Q_PROPERTY(QString bImagePath READ getBImagePath WRITE setBImagePath NOTIFY bImagePathChanged)
    Q_PROPERTY(bool favourite READ isFavourite WRITE setFavourite NOTIFY favouriteChanged)
    Q_PROPERTY(QString filename READ getFilename WRITE setFilename NOTIFY filenameChanged)
    Q_PROPERTY(QString localPath READ getLocalPath WRITE setLocalPath NOTIFY localPathChanged)
public:
    Track(QObject* parent = 0);
    Track(const Track& track);
    virtual ~Track();

    bool operator==(const Track& track);

    Q_INVOKABLE const QString& getId() const;
    Q_INVOKABLE void setId(const QString& id);

    Q_INVOKABLE const int& getDuration() const;
    Q_INVOKABLE void setDuration(const int& duration);

    Q_INVOKABLE const QString& getStreamUrl() const;
    Q_INVOKABLE void setStreamUrl(const QString& streamUrl);

    Q_INVOKABLE const QString& getArtworkUrl() const;
    Q_INVOKABLE void setArtworkUrl(const QString& artworkUrl);

    Q_INVOKABLE const QString& getBArtworkUrl() const;
    Q_INVOKABLE void setBArtworkUrl(const QString& bArtworkUrl);

    Q_INVOKABLE const QString& getTitle() const;
    Q_INVOKABLE void setTitle(const QString& title);

    Q_INVOKABLE const QString& getImagePath() const;
    Q_INVOKABLE void setImagePath(const QString& imagePath);

    Q_INVOKABLE const QString& getBImagePath() const;
    Q_INVOKABLE void setBImagePath(const QString& bImagePath);

    Q_INVOKABLE const bool& isFavourite() const;
    Q_INVOKABLE void setFavourite(const bool& favourite);

    Q_INVOKABLE const QString& getFilename() const;
    Q_INVOKABLE void setFilename(const QString& filename);

    Q_INVOKABLE const QString& getLocalPath() const;
    Q_INVOKABLE void setLocalPath(const QString& localPath);

    Q_INVOKABLE QVariantMap toMap();
    void fromMap(const QVariantMap& map);

    Q_SIGNALS:
        void idChanged(const QString& id);
        void durationChanged(const int& duration);
        void artworkUrlChanged(const QString& artworkUrl);
        void bArtworkUrlChanged(const QString bArtworkUrl);
        void streamUrlChanged(const QString& streamUrl);
        void titleChanged(const QString& title);
        void imagePathChanged(const QString& imagePath);
        void bImagePathChanged(const QString& bImagePath);
        void favouriteChanged(const bool& favourite);
        void filenameChanged(const QString& filename);
        void localPathChanged(const QString& localPath);

private:
    QString m_id;
    int m_duration;
    QString m_artworkUrl;
    QString m_bArtworkUrl;
    QString m_streamUrl;
    QString m_title;
    QString m_imagePath;
    QString m_bImagePath;
    bool m_favourite;
    QString m_filename;
    QString m_localPath;

    void swap(const Track& track);
};

#endif /* TRACK_HPP_ */
