#!/usr/bin/env bash
export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common
cd "$(dirname "$0")"
exec ./build/paired_runner --same-binary 4 --multiscale
