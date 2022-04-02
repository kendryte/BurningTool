#!/usr/bin/env bash

set -Eeuo pipefail

export LD_PRELOAD=/usr/lib64/libasan.so.6

if [[ $# -gt 0 ]] ;then
	exec "$@"
fi

exec ./build-vscode-$HOSTNAME/gui/BurningTool
