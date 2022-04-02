#!/usr/bin/env bash

set -Eeuo pipefail

clang-format -dump-config >/dev/null

find cli/ gui/ library/ test-binary/ -name *.h -o -name *.c -o -name *.cpp | xargs -t -IF clang-format -i "F"

cmake-format -i CMakeLists.txt
cmake-format -i cli/CMakeLists.txt
cmake-format -i gui/CMakeLists.txt
cmake-format -i library/CMakeLists.txt
cmake-format -i test-binary/CMakeLists.txt
