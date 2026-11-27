#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Fork: renders the product's icon (icon.svg, icon-small.svg in the folder
# branding.json's iconDir names) into the PNG files the application loads
# and the Windows icon icon.ico (the program file and the installer). The
# results are committed; run this after changing an SVG. Needs rsvg-convert
# and ImageMagick (install-packages.sh).
#
# Usage: tools/make-brand-icons.sh
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
dir="$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["iconDir"])' "$root/branding.json")"
cd "$root/$dir"

small=(16 20 24 32 40 48)
large=(64 96 128 256)
mkdir -p png
pngs=()
for size in "${small[@]}"; do
  rsvg-convert -w "$size" -h "$size" icon-small.svg -o "png/icon-$size.png"
  pngs+=("png/icon-$size.png")
done
for size in "${large[@]}"; do
  rsvg-convert -w "$size" -h "$size" icon.svg -o "png/icon-$size.png"
  pngs+=("png/icon-$size.png")
done
# Keep each size's own image; no resampling, no metadata (reproducible).
convert "${pngs[@]}" -strip icon.ico
echo "Wrote $dir/png/*.png and $dir/icon.ico"
