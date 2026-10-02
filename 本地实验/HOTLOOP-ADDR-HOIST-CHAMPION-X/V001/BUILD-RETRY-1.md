# V001 Build Retry 1

```text
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
REVISION=V001
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_OFFICIAL=45.16
CANDIDATE_SOURCE_COMMIT=d1de6208d9c52b78c79db59c2cfd1d79ffbe340f
CANDIDATE_SOURCE_SHA256=26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602
BUILD_FIX_COMMIT=b8e8d915a496291503f92934538fb812108b2a94
HOST=hwnput3
DATE_UTC=2026-10-02
BUILD_KIND=LOCAL_HWNPUT3_RETRY_FIXED_CANN_COMPILER
CANN=8.5.0.alpha002
ASC_COMPILER=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng
SERVER3_ALIAS=cann-server3
SERVER3_ACCESS=BLOCKED_DNS_TEMPORARY_FAILURE
```

## Retry result

The retry remains `BUILD_FAILED`:

```text
CMAKE_CONFIGURE=PASS
ASC_COMPILE=FAIL
LINK=NOT_REACHED
BUILD_RC=2
CORRECTNESS=NOT_RUN
LOCAL_PERFORMANCE=NOT_RUN
ONLINE=NOT_RUN
```

The compiler identity fix was active in both the configure log and retry
`CMakeCache.txt`. The first ASC compilation still fails in compiler-generated
host registration code:

```text
/tmp/asc_plugin_binary_register_code-8d93b3.c:3:10: fatal error: 'vector' file not found
#include <vector>
         ^~~~~~~~
```

The following `cceld`, `llvm-objcopy`, `ld.lld`, and `bisheng` messages are
failure cascades after that missing C++ header. No object or linked executable
was produced. This is a local compiler/toolchain blocker; it is not a
Correctness or performance verdict for H3.

## Complete retained evidence

- Preflight and local device/access state:
  `logs/build-retry-preflight-local-hwnput3-20261002.txt`
- CMake configure and pinned compiler:
  `logs/build-retry-configure-local-hwnput3-20261002.txt`
- Complete retry compile/link output and return code:
  `logs/build-retry-compile-local-hwnput3-20261002.txt`
- Retry diagnostic build tree is retained locally but is intentionally not
  committed: `build-v001-retry/`

The retry preflight records `hwnput3`, local `npu-smi`, Ascend 910B3 devices,
the toolkit compiler version, and no process intervention. The requested
current local snapshot was all above the 100 MB compile admission floor:

```text
FREE_HBM_MB=d0:5312,d1:4044,d2:1440,d3:4076,d4:5121,d5:1486,d6:1487,d7:40758
```

`ssh cann-server3` remains unavailable from this runtime with:

```text
ssh: Could not resolve hostname cann-server3: Temporary failure in name resolution
```

## Disposition

The compiler-path retry has been exhausted for this event. V001 is terminal
for this local event as `BUILD_FAILED`; Correctness and all performance gates
are prohibited until a buildable exact source is available. No V002 is
created, and no H1/H2 or other optimization is layered onto V001.
