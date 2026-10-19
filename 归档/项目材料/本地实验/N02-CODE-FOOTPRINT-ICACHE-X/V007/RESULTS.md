# N02-CODE-FOOTPRINT-ICACHE-X V007

ROUTE: N02-CODE-FOOTPRINT-ICACHE-X  
REVISION: V007  
PARENT_EXPERIMENT: R31B V011 / Official 45.16  
PARENT_SOURCE_COMMIT: 7132c86d32e8fa1d57e2ae8edb7846e3b4775b2a  
PREVIOUS_N02_SOURCE_COMMIT: 8eefd778133b449e4e10ca3c35bb6da0d708d099  
CANDIDATE_SOURCE_COMMIT: 84825f8a3715f983ca2d25ec06dc200d056dc32f  
SOURCE_SHA256: 0a2e90b0eae695fb932991bce7fbe79870c2ac469dfe71baf02d99950d4fef40  
BRANCH: route/n02-code-footprint-icache-x-20261011  
REMOTE_HOST: hwnput3  
SOC: Ascend910B3  
ASSIGNED_DEVICE: d6 / NPU 6  
TOOLCHAIN: CANN 8.5.0.alpha002  
REMOTE_RUN: `/home/data4t2/lelinfeng/cann/server_runs/N02-CODE-FOOTPRINT-ICACHE-X-RESET-20261011/V007/0a2e90b0eae695fb932991bce7fbe79870c2ac469dfe71baf02d99950d4fef40`

## Change And Scope

Pass 1 in `ProcessWideLowPrecision` changed from deriving row and tile coordinates from a flattened unit counter to explicit nested row/tile counters. The tile order, ping-pong parity, loads, arithmetic and event sequence were intended to remain unchanged.

This changes device-side scalar index/address calculations as well as loop layout. It overlaps N05 Scalar address generation, so the change does not belong on the N02 axis. Keep the source and partial compile evidence; do not complete V007 or run its Correctness or Local.

## Compile And Device Objects

Parent, Candidate and paired-runner CMake configuration each returned 0. Parent and Candidate device object compilation completed, but each `full_link` returned 2 because the remote input V016 `adapter.cpp` still included `N02-CODE-FOOTPRINT-ICACHE-X/V006/submission.asc`, absent from the V007 run tree. After that attempt, the local staged adapter was changed to include V007; this staged change was not transferred to the remote run directory and no build was repeated. The full-link failure is an input-binding failure, not evidence that V007 links.

The paired Runner build separately returned 2 because CCEC could not locate `<vector>`. No Runner executable was produced. All original configure/build logs and statuses are retained in `results/compile/`.

| Device object | Size |
|---|---:|
| Parent V011 `device.cpp.o` | 334040 B |
| Candidate V007 `device.cpp.o` | 334056 B |

The object files differ byte-for-byte. No linked device `.alink` or paired Runner executable was produced. The objects are retained in `results/artifacts/` as partial compile evidence; their size difference alone does not establish a complete device implementation.

## Correctness And Local

CORRECTNESS: NOT RUN. The paired Runner did not build.  
LOCAL: NOT RUN. No V007 device sampling command was started.  
LOCAL_SCORE: NONE  
LOCAL_DELTA: NONE

## Device Context

Before compilation, d6 reported 59876/65536 MB HBM used, 5660 MB free and 0% AICore. `VLLMWorker_TP` used 56355 MB. The process was left untouched. The N02 run-directory process query returned 1 with no matching process. The timestamp and full `npu-smi` output are in `results/snapshots/pre-compile-*`.

## Disposition

No device correctness or performance conclusion is drawn. The next N02 hypothesis should change generated loop layout without changing address calculation or MTE prefetch conditions/order.
