# Your own catalogue server

Serve your own music to Retrowavers, speaking the same API as retrowave-radio.ru
(documented in [../API.md](../API.md)).

**Almost no backend.** `build-catalog.py` turns a music folder into a JSON file and
a folder of covers; nginx hands them out. The one moving part is the transcoder,
and only because of lossless files — see below.

That the rest can be static comes down to one detail: the `sessionId` in the API
exists so the original server can avoid repeating tracks. The app does not depend
on it — it de-duplicates by track id itself and stops asking for more once a batch
brings nothing new. So a server that answers the same full catalogue every time is
perfectly valid.

## FLAC without converting anything

An mp3 in your library is copied into place. A **FLAC (or m4a/ogg/wav) is not
converted** — the catalogue simply points at `<slug>.mp3`, and the first time
someone actually plays that track, `transcode-service.py` produces it with ffmpeg
at 320 kbps CBR, saves it where nginx looks, and serves it. Every later play is a
plain static file with working seek.

So a few hundred gigabytes of lossless stay exactly as they are. The mp3 cache
grows only to cover what was really listened to, and the originals are read but
never modified. (320 kbps CBR is the top of what mp3 does, and CBR is the safer
choice on old players: seeking does not depend on a Xing header.)

**This means the server needs access to the library**, which is worth thinking
about before choosing where to host — a VPS with a few hundred gigabytes of disk is
expensive, while a machine at home with the library already on it is free. A
reasonable middle ground: run everything at home behind a tunnel, or keep the
originals at home and rsync only the mp3 cache to a small VPS.

## Build the catalogue

```bash
pip install mutagen           # required
pip install Pillow            # optional: shrinks covers to 800 px
apt install ffmpeg            # required only if you have lossless files

python3 build-catalog.py --music ~/music --out /var/www/retrowave
```

What it produces:

```
/var/www/retrowave/
    api/v1/tracks.json           the track list
    api/v1/artwork/<slug>.jpg    covers, taken from the files' own tags
    api/v1/stream/<slug>.mp3     mp3 sources, copied (lossless appears here on first play)
    sources.json                 slug -> original file, for the transcoder
    ids.json                     slug -> id map, see below
```

Title, artist, genre and length come from the file's own tags — ID3, Vorbis
comments or MP4 atoms, whichever it has. The containing folder is added as an extra
tag. If a track has no embedded picture, a `cover.jpg` next to it is used.

**`ids.json` matters.** Favourites are stored on the device under the track id, so
ids must survive a rebuild. The map keeps each slug's id and hands new ids only to
new files — don't delete it, and keep it if you move the server.

## Serve it

```bash
cp nginx.conf.example /etc/nginx/sites-available/retrowave
ln -s /etc/nginx/sites-available/retrowave /etc/nginx/sites-enabled/
nginx -t && systemctl reload nginx

install -d /opt/retrowavers && cp transcode-service.py /opt/retrowavers/
cp retrowavers-transcode.service /etc/systemd/system/
systemctl enable --now retrowavers-transcode
```

Set `server_name` and `root` in the nginx config first (`server_name _;` works if
you have no domain yet). `www-data` needs read access to the music library and
write access to `api/v1/stream`.

Check it before touching the app:

```bash
curl "http://<host>/api/v1/tracks/shuffled?limit=5&sessionId=test"
curl -sI "http://<host>/api/v1/stream/<slug>.mp3" | grep -iE "content-type|accept-ranges"
```

The first must return a JSON array. For a lossless track the second takes a few
seconds the first time (that is ffmpeg) and is instant afterwards — that is the
whole feature working.

## Point the app at it

The endpoints are constants in [`../src/Common.hpp`](../src/Common.hpp):

```cpp
#define RWR_ROOT_ENDPOINT "https://retrowave-radio.ru"
#define RWR_API_ENDPOINT "https://retrowave-radio.ru/api/v1"
```

Change both to your host and rebuild. Note that track ids are namespaced by source
(`rwr:49`), so favourites saved against retrowave-radio.ru would collide with your
catalogue's ids — either clear favourites when switching, or give the source a
different `id()` in `RetrowaveRadioSource` and treat it as a separate source.

## The shape of a track

```json
{"id": 1, "name": "Silicon Gen", "author": "Minemice",
 "tags": ["synthwave"], "duration": 209000,
 "streamUrl": "/api/v1/stream/silicon-gen.mp3",
 "artworkUrl": "/api/v1/artwork/silicon-gen.jpg"}
```

`duration` is **milliseconds**. `streamUrl` and `artworkUrl` are paths, not full
URLs — the app prefixes them with the host. An empty `artworkUrl` is fine: the
player falls back to the default cassette.

## Keeping it fed

Re-run `build-catalog.py` whenever you add music; a cron entry or a systemd timer
saves doing it by hand. The transcoder re-reads `sources.json` per request, so it
picks up new tracks without a restart.

If you ever want the cache warmed instead of filled on demand, just fetch the
tracks: `curl -s -o /dev/null "http://<host>/api/v1/stream/<slug>.mp3"` in a loop
over `tracks.json` does it, at whatever pace you like.

## If you outgrow static files

Real per-listener shuffling, tag filtering, play counts — that is a small service
(~150 lines) reading the same `tracks.json`. The app needs no changes: the contract
stays the same.
