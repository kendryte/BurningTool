#!/usr/bin/env bash

set -Eeuo pipefail

cd "$(dirname "$(realpath "${BASH_SOURCE[0]}")")"

if ! [[ -e .temp ]]; then
	mkdir .temp
fi

ARG=()
for I in 128 96 64 48 32; do
	F=".temp/icon-$I.png"
	ARG+=("$F")
	convert icon.png -resize "${I}x${I}" "$F"
done

convert "${ARG[@]}" ../gui/resources/application.ico
