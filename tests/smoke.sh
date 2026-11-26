#!/bin/sh
# Headless run of the real program, always on a private xvfb-run display
# (never the caller's DISPLAY): raylite must still be running after 4 s, and a
# screenshot of the screen must show the rendered cube, i.e. pixels other than
# the white background that data/test-cubo.scn sets.
set -eu
here=$(cd "$(dirname "$0")/.." && pwd)
command -v xvfb-run >/dev/null || { echo "FAIL: xvfb-run is required (package xvfb)"; exit 1; }
command -v import >/dev/null || { echo "FAIL: import is required (package imagemagick)"; exit 1; }
shot=$(mktemp --suffix=.png)
trap 'rm -f "$shot"' EXIT
cd "$here"   # the scene path is relative to the repository root
env -u DISPLAY -u WAYLAND_DISPLAY xvfb-run -a -s "-screen 0 800x600x24" sh -c \
  './raylite & pid=$!; sleep 4; import -window root "$0"; kill $pid' "$shot" \
  || { echo "FAIL: raylite was not running after 4 s"; exit 1; }
im=magick; command -v magick >/dev/null || im=convert
coloured=$($im "$shot" -fuzz 2% -fill black -opaque white -fill white +opaque black -format '%[fx:mean*w*h]' info:)
echo "non-white pixels: $coloured"
awk -v n="$coloured" 'BEGIN { exit !(n > 500) }' || { echo "FAIL: nothing rendered"; exit 1; }
echo "ok: raylite renders the cube scene under xvfb"
