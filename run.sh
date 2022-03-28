#!/usr/bin/env bash

set -Eeuo pipefail

export LD_PRELOAD=/usr/lib64/libasan.so.6
./build-vscode/test-binary/test
