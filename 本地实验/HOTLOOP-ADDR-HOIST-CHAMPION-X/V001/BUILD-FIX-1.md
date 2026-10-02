# V001 Bounded Build Fix 1 — HCC Include Adapter

```text
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
REVISION=V001
BUILD_FIX_COMMIT=f0f90fad09677aa4b2f9df07531c8c0db9bb9d5d
BUILD_FIX_SCOPE=SUPPORT_ONLY_CMAKE_ADAPTER
FIXED_FILE=本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/CMakeLists.txt
KERNEL_SOURCE_SHA256=26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602
HOST=hwnput3
DATE_UTC=2026-10-02
CANN=8.5.0.alpha002
ASC_COMPILER=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng
BUILD_DIR=本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/build-v001-fix1
```

## Smallest bounded fix

Only `CMakeLists.txt` changed. The adapter now injects the HCC target include
root, C++ 7.3.0 include directory, target-specific C++ include directory,
and `backward` directory in the generated ASC compile command's common and
host portions. No `submission.asc` byte changed, and no H3 kernel mechanism
was edited.

The verbose compile command confirms these paths were present:

```text
-I.../toolkit/toolchain/hcc/aarch64-target-linux-gnu/include
-I.../toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0
-I.../toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu
-I.../toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward
...
-Xhost-start
-I.../toolkit/toolchain/hcc/aarch64-target-linux-gnu/include
-I.../toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0
-I.../toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu
-I.../toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward
...
-Xhost-end
```

## Exact commands and result

Configure command:

```text
bash -lc 'source /usr/local/Ascend/ascend-toolkit/set_env.sh; cmake -S 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001 -B 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/build-v001-fix1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_ASC_COMPILER=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng 2>&1 | tee 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/logs/build-fix1-configure-local-hwnput3-20261002.txt; rc=$?; echo CMAKE_CONFIGURE_PIPE_RC=$rc | tee -a 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/logs/build-fix1-configure-local-hwnput3-20261002.txt; exit $rc'
```

```text
CMAKE_CONFIGURE=PASS
CMAKE_CONFIGURE_RC=0
```

Build command:

```text
bash -lc 'source /usr/local/Ascend/ascend-toolkit/set_env.sh; cmake --build 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/build-v001-fix1 --parallel 1 --verbose 2>&1 | tee 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/logs/build-fix1-compile-local-hwnput3-20261002.txt; rc=$?; echo BUILD_PIPE_RC=$rc | tee -a 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/logs/build-fix1-compile-local-hwnput3-20261002.txt; exit $rc'
```

The raw wrapper appended `BUILD_PIPE_RC=0` because it captured `tee`'s
status. The actual build status is unambiguous in the same complete log:
the final `gmake` commands return `Error 2`. Therefore the recorded build
result is:

```text
CMAKE_CONFIGURE=PASS
ASC_COMPILE=FAIL
LINK=NOT_REACHED
BUILD_RC=2
BUILD_WRAPPER_PIPE_RC=0_NOT_A_BUILD_RESULT
```

The new exact compiler blocker is:

```text
/tmp/asc_plugin_binary_register_code-311efe.c:3:10: fatal error: 'vector' file not found
#include <vector>
         ^~~~~~~~
```

The later `cceld`, `llvm-objcopy`, `ld.lld`, and `bisheng` messages are
cascades after the generated host registration source fails. No ASC object
or executable was produced.

## Retained evidence and disposition

- Configure log:
  `logs/build-fix1-configure-local-hwnput3-20261002.txt`
- Full verbose Build log:
  `logs/build-fix1-compile-local-hwnput3-20261002.txt`
- Earlier Attempt-1 and compiler-pinned retry failures remain intact in
  `logs/build-*.txt` and `logs/build-retry-*.txt`, with their prior commits
  `b8e8d915a496291503f92934538fb812108b2a94` and
  `107a906acc2c4ec7f43ad9f30bcb78b98115b1b8`.
- Diagnostic build tree `build-v001-fix1/` is retained locally but is not
  committed.

This bounded support fix remains `BUILD_FAILED`. `CORRECTNESS=NOT_RUN`,
`LOCAL_PERFORMANCE=NOT_RUN`, and `ONLINE=NOT_RUN`. The missing generated
host `<vector>` remains a toolchain/ASC wrapper blocker even after the HCC
include paths are visible in the command. Stop here pending external/toolchain
handling; do not start performance or create V002.
