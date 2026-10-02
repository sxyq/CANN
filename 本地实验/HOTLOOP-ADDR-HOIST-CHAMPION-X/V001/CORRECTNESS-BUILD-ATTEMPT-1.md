# V001 Correctness Harness Build Attempt 1 — FAIL

```text
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
REVISION=V001
HARNESS_COMMIT=3573e6cee48d906ee556d3de63f73f61761f84cf
CANDIDATE_SOURCE_SHA256=26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
HOST=hwnput3
DEVICE_PLATFORM=Ascend910B3
DATE_UTC=2026-10-02
CANN=8.5.0.alpha002
BUILD_DIR=本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/build-correctness
```

## Exact result

```text
CMAKE_CONFIGURE=PASS
CMAKE_CONFIGURE_RC=0
ASC_COMPILE=FAIL
LINK=NOT_REACHED
BUILD_RC=2
CORRECTNESS=NOT_RUN
LOCAL_PERFORMANCE=NOT_STARTED
```

The first support harness build failed in `correctness_runner.cpp:149` because
the ASC compiler rejected `static_cast<void*>` to its `__gm__ uint8_t*` address
space type. This is a correctness-runner support cast only; `submission.asc`
and the H3 implementation were unchanged.

## Exact commands and logs

```text
set +u; source /usr/local/Ascend/ascend-toolkit/set_env.sh; set -u
cmake -S 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/correctness -B 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/build-correctness -DCMAKE_BUILD_TYPE=Release -DCMAKE_ASC_COMPILER=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng
cmake --build 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/build-correctness --parallel 1 --verbose
```

Logs:

- `logs/correctness-configure-local-hwnput3-20261002.txt`
- `logs/correctness-build-local-hwnput3-20261002.txt`

The bounded next event is a support-only C-style address cast fix followed by
an immediate rebuild. No timing or Online action is allowed from this result.
