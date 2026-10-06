#!/usr/bin/env python3
"""
build-catalog.py - turn a music folder into a Retrowavers catalogue.

Produces what the app asks for (see ../API.md):

    <out>/api/v1/tracks.json          the track list
    <out>/api/v1/artwork/<slug>.jpg   cover art, taken from the file's own tags
    <out>/api/v1/stream/<slug>.mp3    the audio

The last one is where lossless libraries are handled: an mp3 source is copied as
is, but a FLAC (or m4a/ogg/wav) is NOT converted here. It is recorded in
sources.json, and transcode-service.py turns it into a 320 kbps mp3 the first time
someone actually plays it. So a few hundred gigabytes of FLAC stay untouched, and
the mp3 cache only ever grows to cover what was really listened to.

Track ids are kept STABLE across runs (see ids.json): favourites on the device are
stored under them, so a rebuilt catalogue must not renumber anything.

Usage:
    pip install mutagen           # and optionally Pillow, to shrink covers
    python3 build-catalog.py --music ~/music --out /var/www/retrowave

Re-run it whenever you add music.
"""

import argparse
import json
import os
import re
import shutil
import sys

# Sources we can serve. mp3 is copied; everything else is transcoded on demand.
LOSSLESS_EXT = (".flac", ".m4a", ".ogg", ".oga", ".wav", ".aiff", ".aif")
ALL_EXT = (".mp3",) + LOSSLESS_EXT

# The app downscales covers to 720 px anyway, so anything bigger is wasted traffic.
COVER_MAX_PX = 800
COVER_QUALITY = 85


def fail(message):
    sys.stderr.write("error: " + message + "\n")
    sys.exit(1)


def load_mutagen():
    try:
        import mutagen
        return mutagen
    except ImportError:
        fail("mutagen is missing - install it with: pip install mutagen")


def slugify(name):
    """A filename safe on any filesystem and inside a URL, kept readable."""
    slug = name.lower()
    slug = re.sub(r"[^a-z0-9._-]+", "-", slug)
    slug = re.sub(r"-+", "-", slug).strip("-.")
    return slug or "track"


def first_value(audio, keys):
    """First non-empty value among `keys`. Tag names differ per container."""
    for key in keys:
        try:
            value = audio.get(key)
        except Exception:
            value = None
        if not value:
            continue
        if isinstance(value, list):
            value = value[0]
        text = str(value).strip()
        if text:
            return text
    return ""


def read_meta(audio):
    """title, artist, genre - across ID3 (mp3), Vorbis (flac/ogg) and MP4 tags."""
    title = first_value(audio, ["TIT2", "title", "\xa9nam"])
    artist = first_value(audio, ["TPE1", "artist", "\xa9ART"])
    genre = first_value(audio, ["TCON", "genre", "\xa9gen"])
    return title, artist, genre


def extract_cover(audio, target_path):
    """Write the embedded cover to target_path. True if one was found."""
    data = None

    # FLAC / OGG: a list of picture blocks.
    pictures = getattr(audio, "pictures", None)
    if pictures:
        data = pictures[0].data

    # MP3: an APIC frame. MP4: the 'covr' atom.
    if data is None:
        try:
            for key in audio.keys():
                if key.startswith("APIC"):
                    data = audio[key].data
                    break
                if key == "covr" and audio[key]:
                    data = bytes(audio[key][0])
                    break
        except Exception:
            data = None

    if not data:
        return False

    with open(target_path, "wb") as handle:
        handle.write(data)
    shrink_cover(target_path)
    return True


def shrink_cover(path):
    """Downscale a cover in place, if Pillow is available. Optional by design."""
    try:
        from PIL import Image
    except ImportError:
        return

    try:
        image = Image.open(path)
        if max(image.size) <= COVER_MAX_PX:
            return
        image.thumbnail((COVER_MAX_PX, COVER_MAX_PX), Image.LANCZOS)
        image.convert("RGB").save(path, "JPEG", quality=COVER_QUALITY)
    except Exception as exc:
        sys.stderr.write("warn: could not shrink %s (%s)\n" % (path, exc))


def load_json(path, fallback):
    if not os.path.exists(path):
        return fallback
    with open(path, "r") as handle:
        return json.load(handle)


def save_json(path, data, sort_keys=False):
    with open(path, "w") as handle:
        json.dump(data, handle, ensure_ascii=False, indent=1, sort_keys=sort_keys)


def next_id(ids):
    return (max(ids.values()) + 1) if ids else 1


def collect_tags(genre, folder):
    """The file's genre, plus the folder it sits in - both are useful as tags."""
    tags = []
    for raw in re.split(r"[;,/]", genre):
        tag = raw.strip().lower()
        if tag and tag not in tags:
            tags.append(tag)

    folder_tag = folder.strip().lower()
    if folder_tag and folder_tag not in tags:
        tags.append(folder_tag)

    return tags


def build(music_dir, out_dir):
    mutagen = load_mutagen()

    api_dir = os.path.join(out_dir, "api", "v1")
    stream_dir = os.path.join(api_dir, "stream")
    artwork_dir = os.path.join(api_dir, "artwork")
    for directory in (stream_dir, artwork_dir):
        if not os.path.isdir(directory):
            os.makedirs(directory)

    ids_path = os.path.join(out_dir, "ids.json")
    ids = load_json(ids_path, {})
    sources = {}

    tracks = []
    used_slugs = set()
    lossless = 0

    for root, dirs, files in os.walk(music_dir):
        for filename in sorted(files):
            ext = os.path.splitext(filename)[1].lower()
            if ext not in ALL_EXT:
                continue

            source = os.path.join(root, filename)
            base = os.path.splitext(filename)[0]
            slug = slugify(base)

            # Two files can slugify to the same name; keep both.
            suffix = 2
            while slug in used_slugs:
                slug = "%s-%d" % (slugify(base), suffix)
                suffix += 1
            used_slugs.add(slug)

            try:
                audio = mutagen.File(source)
                if audio is None:
                    raise ValueError("unsupported file")
                duration_ms = int(round(audio.info.length * 1000))
            except Exception as exc:
                sys.stderr.write("warn: skipping %s (%s)\n" % (source, exc))
                continue

            title, artist, genre = read_meta(audio)
            folder = os.path.basename(root) if os.path.abspath(root) != os.path.abspath(music_dir) else ""

            if ext == ".mp3":
                # Already the format we serve - copy it and be done.
                shutil.copyfile(source, os.path.join(stream_dir, slug + ".mp3"))
            else:
                # Leave the lossless master where it is; transcode-service.py will
                # produce <slug>.mp3 the first time this track is played.
                sources[slug] = os.path.abspath(source)
                lossless += 1

            artwork_url = ""
            cover_path = os.path.join(artwork_dir, slug + ".jpg")
            if extract_cover(audio, cover_path):
                artwork_url = "/api/v1/artwork/" + slug + ".jpg"
            else:
                # No embedded picture: fall back to a cover.jpg next to the file.
                sibling = os.path.join(root, "cover.jpg")
                if os.path.exists(sibling):
                    shutil.copyfile(sibling, cover_path)
                    shrink_cover(cover_path)
                    artwork_url = "/api/v1/artwork/" + slug + ".jpg"

            if slug not in ids:
                ids[slug] = next_id(ids)

            tracks.append({
                "id": ids[slug],
                "name": title or base,
                "author": artist or "Unknown",
                "tags": collect_tags(genre, folder),
                "duration": duration_ms,          # milliseconds, as the app expects
                "streamUrl": "/api/v1/stream/" + slug + ".mp3",
                "artworkUrl": artwork_url,
            })

    tracks.sort(key=lambda track: track["id"])

    save_json(os.path.join(api_dir, "tracks.json"), tracks)
    save_json(ids_path, ids, sort_keys=True)
    save_json(os.path.join(out_dir, "sources.json"), sources, sort_keys=True)

    covers = len([t for t in tracks if t["artworkUrl"]])
    print("catalogue: %d tracks (%d transcoded on demand), %d with cover art"
          % (len(tracks), lossless, covers))
    print("           -> %s" % os.path.join(api_dir, "tracks.json"))


def main():
    parser = argparse.ArgumentParser(description="Build a Retrowavers catalogue from a music folder.")
    parser.add_argument("--music", required=True, help="folder to scan (searched recursively)")
    parser.add_argument("--out", required=True, help="web root to write into")
    args = parser.parse_args()

    if not os.path.isdir(args.music):
        fail("no such music folder: " + args.music)

    build(args.music, args.out)


if __name__ == "__main__":
    main()
