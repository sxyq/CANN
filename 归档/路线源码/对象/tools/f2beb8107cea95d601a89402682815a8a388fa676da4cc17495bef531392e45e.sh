#!/usr/bin/env bash
set -euo pipefail

: "${ASCEND_HOME_PATH:?ASCEND_HOME_PATH is required}"
cmake -S . -B build
cmake --build build -j2
