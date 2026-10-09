# Build Result

ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V001
STAGE: BUILD
STATUS: PASS
HOST: hwnput3 (aarch64)
TOOLCHAIN: `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002` kernel target; `/usr/local/Ascend/ascend-toolkit/latest` paired target
TARGETS: `crossrow_v001`; `crossrow_v001_parent`; `crossrow_v001_candidate`; `crossrow_v001_paired_runner`

## Final compile invocation

The existing V012 CMake targets were built serially with the project toolkit environment and the HCC C++ include roots required by the local plugin:

```text
source /usr/local/Ascend/ascend-toolkit/set_env.sh
CPLUS_INCLUDE_PATH=<HCC C++ include roots> CPATH=<HCC C++ include roots> cmake --build 本地实验/CROSSROW-EPI-PROLOGUE-X/V001/build --parallel 1
CPLUS_INCLUDE_PATH=<HCC C++ include roots> CPATH=<HCC C++ include roots> cmake --build 本地实验/CROSSROW-EPI-PROLOGUE-X/V001/support/build --parallel 1
```

Final output is retained in `compile-v001-final.log`. Earlier environment-only failures are retained in `compile-v001-local.log`, `compile-v001-retry.log`, and `compile-v001-retry-latest.log`; the successful include-environment retry is retained in `compile-v001-retry-includes.log`.

BUILD_RC: 0
