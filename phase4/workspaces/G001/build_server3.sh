#!/usr/bin/env bash
set -eu
g001_root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
set +u
source /usr/local/Ascend/ascend-toolkit/set_env.sh
set -u
export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
export CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}
cmake -S "$g001_root" -B "$g001_root/build" \
    -DSOC_VERSION=Ascend910B3 -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$g001_root/build"
cmake --build "$g001_root/build" -j2
