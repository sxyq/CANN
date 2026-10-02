# V001 Bounded Build Fix 2 — Build PASS

```text
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
REVISION=V001
BUILD_FIX_COMMIT=8ef756a385d57548f447ded85964a5a0bcb947ec
BUILD_FIX_SCOPE=SUPPORT_ONLY_ASCPLUGIN_INCLUDE_ENV
CANDIDATE_SOURCE_COMMIT=d1de6208d9c52b78c79db59c2cfd1d79ffbe340f
CANDIDATE_SOURCE_SHA256=26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602
HOST=hwnput3
DEVICE_PLATFORM=Ascend910B3
DATE_UTC=2026-10-02
CANN=8.5.0.alpha002
ASC_COMPILER=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng
BUILD_DIR=本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/build-v001-fix2
```

## Result

The bounded environment propagation fix passed the complete local build:

```text
CMAKE_CONFIGURE=PASS
CMAKE_CONFIGURE_RC=0
ASC_COMPILE=PASS
LINK=PASS
BUILD_RC=0
CORRECTNESS=NOT_RUN
LOCAL_PERFORMANCE=NOT_STARTED
ONLINE=NOT_RUN
```

The verbose compile command contains both `CPLUS_INCLUDE_PATH` and
`CPLUSPLUS_INCLUDE_PATH`, each carrying the HCC include root, C++ 7.3.0
directory, target-specific C++ directory, and `backward`. The generated
ASCPLUGIN host compilation completed and CMake reported:

```text
[100%] Built target addr_v001_h3
BUILD_RC=0
```

## Exact commands and evidence

Configure:

```text
bash -lc 'set -o pipefail; source /usr/local/Ascend/ascend-toolkit/set_env.sh; cmake -S 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001 -B 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/build-v001-fix2 -DCMAKE_BUILD_TYPE=Release -DCMAKE_ASC_COMPILER=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng 2>&1 | tee 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/logs/build-fix2-configure-local-hwnput3-20261002.txt; rc=${PIPESTATUS[0]}; echo CMAKE_CONFIGURE_RC=${rc} | tee -a 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/logs/build-fix2-configure-local-hwnput3-20261002.txt; exit ${rc}'
```

Build:

```text
bash -lc 'set -o pipefail; source /usr/local/Ascend/ascend-toolkit/set_env.sh; cmake --build 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/build-v001-fix2 --parallel 1 --verbose 2>&1 | tee 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/logs/build-fix2-compile-local-hwnput3-20261002.txt; rc=${PIPESTATUS[0]}; echo BUILD_RC=${rc} | tee -a 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/logs/build-fix2-compile-local-hwnput3-20261002.txt; exit ${rc}'
```

Logs:

- `logs/build-fix2-configure-local-hwnput3-20261002.txt`
- `logs/build-fix2-compile-local-hwnput3-20261002.txt`

Executable identity:

```text
PATH=build-v001-fix2/addr_v001_h3
SHA256=2b2390ae67e74cc3474e9a0cf77f496628feb825bdd99891f2903157d0d506c8
FILE=ELF 64-bit LSB pie executable, ARM aarch64, dynamically linked, not stripped
SIZE_BYTES=429448
```

The build directory and executable are retained as local diagnostic/build
artifacts and are intentionally not committed. The candidate source remains
H3-only; `submission.asc` was not changed by either support fix.

## Gate transition

Build is PASS. Correctness is the immediate next event. No local timing,
profiling, or Online action has started.
