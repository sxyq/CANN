# N02-CODE-FOOTPRINT-ICACHE-X V008

ROUTE: N02-CODE-FOOTPRINT-ICACHE-X  
REVISION: V008  
PARENT: R31B V011 / Official 45.16  
PARENT_SOURCE_COMMIT: 7132c86d32e8fa1d57e2ae8edb7846e3b4775b2a  
PREVIOUS_N02_SOURCE_COMMIT: 8eefd778133b449e4e10ca3c35bb6da0d708d099  
CANDIDATE_SOURCE_COMMIT: d5739dc618126766d96ab83356858ef2fc82438f  
SOURCE_SHA256: e9dcfed6dd60db2e533fdf5b20be1c49d3e185049373a1d9013bdf09068ae98e  
REMOTE_HOST: hwnput3  
SOC: Ascend910B3  
DEVICE_ASSIGNMENT: d6 / NPU 6  
TOOLCHAIN: CANN 8.5.0.alpha002  
REMOTE_RUN: `/home/data4t2/lelinfeng/cann/server_runs/N02-CODE-FOOTPRINT-ICACHE-X-RESET-20261011/V008/e9dcfed6dd60db2e533fdf5b20be1c49d3e185049373a1d9013bdf09068ae98e`

## Change

Added `#pragma clang loop unroll_count(2)` to the flattened pass-1 unit loop in `ProcessWideLowPrecision`. The intended source-level difference was limited to a compiler unroll hint; flattened indexing, address calculations, DataCopy calls, prefetch condition/order, arithmetic, and event sequence were unchanged.

## Compile And Device Artifacts

Parent and Candidate `full_link`, and the existing paired Runner build, all returned 0 under CANN 8.5.0.alpha002. `llvm-objdump` and symbol extraction returned 0 for both objects.

| Artifact | Parent V011 | Candidate V008 | Difference |
|---|---:|---:|---:|
| `device.cpp.o` | 334040 B | 333912 B | -128 B |
| `device.alink` | 183144 B | 183144 B | 0 B |
| FP16 `ProcessWideLowPrecision` | 0x1234 B | 0x1234 B | 0 B |
| BF16 `ProcessWideLowPrecision` | 0x1248 B | 0x1214 B | -0x34 B |

The Parent/Candidate differences above were already present in V006. V008 Candidate `device.cpp.o` is byte-identical to the V006 Candidate object, and Candidate `device.alink` is byte-identical to the V006 Candidate linked artifact. The compiler did not turn the pragma into a new device implementation. The raw objects, linked artifacts, symbol tables, disassembly, and build logs are retained under `results/`.

## Correctness And Local

CORRECTNESS: NOT RUN. V008 produced the same Candidate device objects as V006.  
LOCAL: NOT RUN. No V008 device sampling command was started.  
LOCAL_SCORE: NONE  
LOCAL_DELTA: NONE

## Disposition

Negative code-generation result: the source hint caused no device object or linked-artifact change. No device result is attributed to V008. Continue with explicit source loop expansion, preserving the flattened index and DataCopy/prefetch order; classify it as mixed if emitted MTE scheduling changes.
