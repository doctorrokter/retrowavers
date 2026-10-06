#!/usr/bin/env python3
"""
transcode-service.py - makes an mp3 out of a lossless track, once, on first play.

nginx serves /api/v1/stream/<slug>.mp3 straight from disk when the file is there,
and falls back to this service when it is not (see nginx.conf.example). We look the
slug up in sources.json, run ffmpeg over the original, save the result where nginx
expects it, and serve it. Every later request for that track never reaches us.

So a lossless library is never converted in bulk: the mp3 cache grows only to cover
what was actually listened to, and the originals are read but never modified.

    python3 transcode-service.py --root /var/www/retrowave [--port 8080] [--bitrate 320k]

Needs ffmpeg on PATH. No Python dependencies - the standard library only.
"""

import argparse
import json
import os
import re
import subprocess
import sys
import threading

try:
    from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
except ImportError:
    sys.stderr.write("error: this service needs python3\n")
    sys.exit(1)

STREAM_PREFIX = "/api/v1/stream/"
CHUNK = 64 * 1024

# One lock per slug: two players starting the same track must not run two ffmpegs
# over the same file, and the second one must wait rather than get a truncated mp3.
_locks = {}
_locks_guard = threading.Lock()


def lock_for(slug):
    with _locks_guard:
        if slug not in _locks:
            _locks[slug] = threading.Lock()
        return _locks[slug]


class Config(object):
    def __init__(self, root, bitrate):
        self.root = root
        self.bitrate = bitrate
        self.stream_dir = os.path.join(root, "api", "v1", "stream")
        self.sources_path = os.path.join(root, "sources.json")

    def sources(self):
        # Re-read per request: the catalogue is rebuilt whenever music is added,
        # and this service should not need restarting for that.
        try:
            with open(self.sources_path, "r") as handle:
                return json.load(handle)
        except Exception as exc:
            sys.stderr.write("warn: cannot read %s (%s)\n" % (self.sources_path, exc))
            return {}


def transcode(source, target, bitrate):
    """FLAC -> mp3. Writes to a temp file and renames, so nginx never sees a partial."""
    temp = target + ".part"

    command = [
        "ffmpeg", "-nostdin", "-loglevel", "error", "-y",
        "-i", source,
        "-vn",                       # cover art is served separately
        "-map_metadata", "0",        # keep title/artist/album for the device library
        "-codec:a", "libmp3lame",
        "-b:a", bitrate,             # CBR: seeking on old players does not depend on a Xing header
        temp,
    ]

    try:
        subprocess.check_call(command)
    except Exception as exc:
        sys.stderr.write("error: ffmpeg failed for %s (%s)\n" % (source, exc))
        if os.path.exists(temp):
            os.remove(temp)
        return False

    os.rename(temp, target)   # atomic: the file appears complete or not at all
    return True


class Handler(BaseHTTPRequestHandler):
    config = None

    def log_message(self, fmt, *args):
        sys.stderr.write("transcode: " + (fmt % args) + "\n")

    def do_HEAD(self):
        self.serve(head_only=True)

    def do_GET(self):
        self.serve(head_only=False)

    def serve(self, head_only):
        path = self.path.split("?")[0]
        if not path.startswith(STREAM_PREFIX) or not path.endswith(".mp3"):
            self.send_error(404)
            return

        slug = path[len(STREAM_PREFIX):-len(".mp3")]
        if not re.match(r"^[A-Za-z0-9._-]+$", slug):
            self.send_error(400)     # no path traversal, no surprises
            return

        target = os.path.join(self.config.stream_dir, slug + ".mp3")

        if not os.path.exists(target):
            source = self.config.sources().get(slug)
            if not source or not os.path.exists(source):
                self.log_message("no source for %s", slug)
                self.send_error(404)
                return

            with lock_for(slug):
                # Another request may have finished it while we waited.
                if not os.path.exists(target):
                    self.log_message("transcoding %s", slug)
                    if not transcode(source, target, self.config.bitrate):
                        self.send_error(500)
                        return

        self.send_file(target, head_only)

    def send_file(self, path, head_only):
        size = os.path.getsize(path)
        start = 0
        end = size - 1
        partial = False

        # Players ask for ranges, both to seek and to resume. nginx does this for
        # cached files; we have to do it ourselves for the first play.
        header = self.headers.get("Range")
        if header:
            match = re.match(r"bytes=(\d*)-(\d*)", header.strip())
            if match:
                first, last = match.group(1), match.group(2)
                if first:
                    start = int(first)
                    if last:
                        end = int(last)
                elif last:
                    start = max(0, size - int(last))
                partial = start > 0 or end < size - 1

        if start >= size:
            self.send_response(416)
            self.send_header("Content-Range", "bytes */%d" % size)
            self.end_headers()
            return

        length = end - start + 1

        self.send_response(206 if partial else 200)
        self.send_header("Content-Type", "audio/mpeg")
        self.send_header("Content-Length", str(length))
        self.send_header("Accept-Ranges", "bytes")
        if partial:
            self.send_header("Content-Range", "bytes %d-%d/%d" % (start, end, size))
        self.end_headers()

        if head_only:
            return

        with open(path, "rb") as handle:
            handle.seek(start)
            remaining = length
            while remaining > 0:
                chunk = handle.read(min(CHUNK, remaining))
                if not chunk:
                    break
                try:
                    self.wfile.write(chunk)
                except Exception:
                    return   # player hung up (skipped the track); nothing to report
                remaining -= len(chunk)


def main():
    parser = argparse.ArgumentParser(description="Transcode lossless tracks to mp3 on first play.")
    parser.add_argument("--root", required=True, help="web root (the --out of build-catalog.py)")
    parser.add_argument("--port", type=int, default=8080)
    parser.add_argument("--bitrate", default="320k", help="mp3 bitrate, default 320k CBR")
    args = parser.parse_args()

    config = Config(args.root, args.bitrate)
    if not os.path.isdir(config.stream_dir):
        sys.stderr.write("error: no stream dir at %s - run build-catalog.py first\n" % config.stream_dir)
        sys.exit(1)

    Handler.config = config
    server = ThreadingHTTPServer(("127.0.0.1", args.port), Handler)
    sys.stderr.write("transcode: listening on 127.0.0.1:%d, cache %s, bitrate %s\n"
                     % (args.port, config.stream_dir, args.bitrate))
    server.serve_forever()


if __name__ == "__main__":
    main()
