# Retrowavers — where the music comes from

The original retrowave.ru died. Version 3 talks to a successor for its catalogue and
to several radio services for live stations. This file records what those APIs
actually do, including the parts that are not obvious from their docs (they have
none) and the parts that bit us.

Everything here was probed against the live services on 2026-09-08.

---

## 1. Architecture in one paragraph

`ApiController` is a facade. It owns a list of `ITrackSource` implementations and
delegates to whichever is active; QML only ever sees `load()` and the `loaded`
signal, exactly as it did in version 2. Every source hands back tracks as
`QVariantMap`s in the shape `Track::fromMap()` expects, so the model and the UI are
source-agnostic.

A source declares its `kind()`:

- **`catalog`** — a library of tracks. The Playlist tab.
- **`radio`** — a service whose entries are live stations. The Radio tab lists the
  *services* first; picking one shows only that service's stations.

Track ids are namespaced by source (`rwr:49`, `soma:vaporwaves`, `legacy:49`), which
is what keeps two sources — and favourites saved by version 2.x — from colliding.

**Adding a source** means writing one class and registering it in
`ApiController`'s constructor. No UI change is needed.

---

## 2. Catalogue: retrowave-radio.ru

Base `https://retrowave-radio.ru/api/v1`. Implemented by `RetrowaveRadioSource`.

| Endpoint | Notes |
|---|---|
| `GET /tracks/shuffled?limit=N&sessionId=S` | The one we use. `sessionId` is **required** — without it, 400 `MISSING_PARAM`. |
| `GET /tracks?limit=N` | Same shape, defaults to 10. `offset`/`page` are ignored. |
| `GET /tracks/{id}` | A single track. |
| `GET /tracks/by-tag?tags=a,b&limit=N&sessionId=S` | Filter by tag. |
| `GET /tags` | 24 tags with track counts. |
| `GET /stream/<uuid>.mp3` | `audio/mpeg`, ~5 MB, supports Range. |
| `GET /artwork/<uuid>.(png\|jpeg)` | **1254×1254 or 2048×2048, 1.8–5.6 MB.** No resize parameters. |
| `GET /hls/live.m3u8` | A LIVE broadcast — see §5. |

A track:

```json
{"id":49,"name":"Synaptic Gavel","author":"Retro Dance Machine",
 "tags":["synthwave"],"duration":209000,
 "streamUrl":"/api/v1/stream/<uuid>.mp3",
 "artworkUrl":"/api/v1/artwork/<uuid>.png"}
```

### Three things worth knowing

**`sessionId` replaces the cursor.** There is no paging. Within one session the
server never repeats a track until the whole catalogue is exhausted, then it starts
over. We generate a UUID per app run and dedupe by id; an empty batch therefore
means "you have seen everything".

**The catalogue is ~94 tracks** (ids 3..101 at the time of writing). `limit=1000`
returns 94. The player wraps to the start when a load brings nothing new.

**Artwork is huge.** Multi-megabyte PNGs the list does not even display, which is
why fetching is lazy (only the playing track) and why `ArtworkProcessor` renders a
720 px cover plus a 160 px blur and then **deletes the original**.

### The ID3 tags in these files are junk

The served mp3s already carry ID3v2.3 tags, but they cannot be trusted. Of three
sampled tracks: one had `TPE1` = `retrowave-radio.ru`, one had neither title nor
artist, and a third named an artist the API disagrees with. All carry a
`made with suno` comment. That is why `MusicLibrary` **rebuilds** the tag from what
the API says before writing a downloaded track into the device music library.

---

## 3. Radio services

Six services, each with its own station list. All streams are plain **http** except
where noted — mm-renderer fetches the stream itself and never sees the CA bundle
installed in `main()`, so http sidesteps TLS entirely.

| Service | Stations | Stream | "Now playing" | Artwork |
|---|---|---|---|---|
| SomaFM | 46 | `http://ice1.somafm.com/<id>-128-mp3` | `https://somafm.com/songs/<id>.xml` | station logo only |
| Nightride FM | 11 | `http://stream.nightride.fm/<id>.mp3` | `https://nightride.fm/meta` (SSE) | none |
| WaveRadio | 3 | `http://station.waveradio.org/<mount>` | `status-json.xsl` | none |
| Nightwave Plaza | 1 | `http://radio.plaza.one/mp3` | `https://api.plaza.one/status` | **per track** |
| SynthwaveRadio.eu | 1 | `https://…/listen/synthwaveradio.eu/radio.mp3` | `/api/nowplaying/synthwaveradio.eu` | **per track** |
| Retrowave.One | 1 | `http://77.108.192.88:8000/stream` | `status-json.xsl` | none |

Metadata is polled every 30 s, and **only while one of that service's stations is
playing** (`ITrackSource::setActiveTrack` starts and stops the timer).

### Per-service quirks

**SomaFM** — `channels.json` returns all 46 stations in one request, with 512 px
logos. The JSON only offers `.pls` playlists, which would mean a request per
station; the direct `ice1…-128-mp3` form was verified against several channels and
is used instead.

**Nightride FM** — no station-list endpoint exists, so the list is hardcoded (it
also gives proper display names). `/meta` is a **server-sent event stream that never
ends**: it opens with a snapshot of every station, so we read until our station
appears, then abort the request. An 8 s watchdog covers a silent stream.

**WaveRadio** — mounts are hardcoded because `status-json.xsl` also lists
low-bitrate and legacy duplicates of each station, which would appear as separate
rows. `witch` is AAC (no mp3 twin); the others are mp3. Carries **Sovietwave**,
which nothing else here does.

**Nightwave Plaza** — `/status` also reports `length`, `position` and `listeners`,
so this is the only station where a real progress bar would be possible. Note
`/mp2` (quoted in some write-ups) **does not exist** — 404; the mounts are `/mp3`
and `/ogg`.

**SynthwaveRadio.eu** — AzuraCast. Its stream is https only (plain http 301s), so
this is the one radio that does not get the "no TLS" benefit.

**Retrowave.One** — ⚠️ the stream lives on a **bare IP**; no hostname resolves to it
(`stream.`/`radio.retrowave.one` do not exist). The address is baked into
`RetrowaveOneSource.cpp`. If the host moves, this station goes silent and the
constant must be updated. It is the only such assumption in the app.

---

## 4. Migrating favourites from version 2.x

Run once, guarded by the `migrated_v3` setting:

- ids get a `legacy:` prefix — old ones were bare numbers, and so are
  retrowave-radio.ru's, so a favourite could otherwise shadow a new track;
- `bArtworkUrl` / `bImagePath` are cleared (the blur service is gone; blurs are
  rendered on device now);
- `artworkUrl` / `streamUrl` pointing at retrowave.ru are cleared — only the
  downloaded file can still play such a track;
- leftover `b_*.png` blurs are deleted from the image cache.

---

## 5. Not implemented, but available

**A LIVE broadcast for the catalogue.** `/api/v1/hls/live.m3u8` is a byte-range HLS
playlist carrying a custom line with the current track and a listener count:

```
#RETRO-META:{"id":94,"name":"Last Pattern","author":"Oscillator 3",
             "artwork":"/api/v1/artwork/<uuid>.png","duration":262608,"listeners":3}
```

mm-renderer is unlikely to handle byte-range HLS, but the broadcast could be
emulated: poll the playlist, read `#RETRO-META`, play the named track from the right
offset. That is a new screen, hence deferred.

**A second catalogue.** Searched and rejected for now: Internet Archive has the
volume (~7200 synthwave/retrowave audio items, no API key) but file paths must be
built from `server`+`dir` in the metadata response and the material is a grab bag;
Jamendo has a clean genre-searchable catalogue but needs a client_id; SoundCloud and
Bandcamp no longer offer public APIs. Radio services expose only recent history, not
a catalogue.
