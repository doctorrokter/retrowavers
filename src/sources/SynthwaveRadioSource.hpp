/*
 * SynthwaveRadioSource - synthwaveradio.eu, a single station running AzuraCast.
 *
 * Worth having for two reasons: the genre is exactly this app's, and AzuraCast's
 * /api/nowplaying reports the ARTWORK of the current track - so, like Nightwave
 * Plaza, this station gets a real cassette and a real blurred backdrop.
 *
 * Its stream is https only (plain http answers 301), which makes it the one radio
 * that cannot claim the "http sidesteps TLS" advantage the others have. Fine in
 * practice: the device already plays the https catalogue.
 */

#ifndef SYNTHWAVERADIOSOURCE_HPP_
#define SYNTHWAVERADIOSOURCE_HPP_

#include "RadioSource.hpp"

class SynthwaveRadioSource: public RadioSource {
    Q_OBJECT
public:
    SynthwaveRadioSource(QObject* parent = 0);

    QString id() const;
    QString name() const;

protected:
    void requestStations();
    QString nowPlayingUrl(const QString& stationKey) const;
    bool parseNowPlaying(const QByteArray& body, const QString& stationKey,
                         QVariantMap& fields) const;
};

#endif /* SYNTHWAVERADIOSOURCE_HPP_ */
