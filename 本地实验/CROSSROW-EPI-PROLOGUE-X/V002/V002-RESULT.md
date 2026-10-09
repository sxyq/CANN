# CROSSROW-EPI-PROLOGUE-X V002 Result

AGENT_ID: W5-R01 CROSSROW-EPI-PROLOGUE-X
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V002
STATUS: LOCAL_NO_PROMOTION_MEASUREMENT_CONTAMINATED

## Selected single factor

HYPOTHESIS_ID: H1_MID_EPILOGUE_ISSUE
DIRECT_PARENT: W5-R01 CROSSROW-EPI-PROLOGUE-X V001 Candidate
DIRECT_PARENT_SOURCE_SHA256: 3698c229dfb6a4b336e7ee0506eb1ceaf898c1c37c937c0af7d3eef71fbc685d
CANDIDATE_SOURCE_SHA256: 426489bfdc63441f2fd179fdff176875b24d815e6e0420343aeed0b0dab7933f
SOURCE_DIFF: `V002-OFAT.diff`

The only Kernel change moves the existing next-row first-tile x/residual prologue from the V001 pre-epilogue position to after current-row epilogue tile zero and before tile one. No arithmetic, ABI, buffer allocation, event allocation, output store, dispatch predicate, or input byte count changed.

The target is the generic cached-row FP16 path at `rows=16,width=6144,blocks=8`, with two 4096-element tiles per row and two rows per effective block. The `M=1` control has localRows=1, so the successor guard is false and no prologue is issued.

This is not W3 V012 pass-2 parameter double buffering, Main-1 CASE47 narrow scalar handoff, CASE14 same-row D-slice overlap, or Tiny fixed overhead.

## Existing runner reuse

RUNNER: existing `本地实验/CROSSROW-EPI-PROLOGUE-X/V001/support/build/crossrow_v001_paired_runner`
NEW_RUNNER: NO
EXECUTION_PATH: existing V001 support CMake/build entry

For the V002 measurement only, the existing runner adapters were temporarily pointed at V002 Parent/Candidate source and its metadata labels were changed to V001/V002. Those three V001 support files were restored after the measurement and the existing V001 entry was rebuilt successfully. No temporary adapter or runner change remains in the worktree.

## Build

BUILD_RC: 0
BUILD_LOG: `compile-v002.log`
ARTIFACT_SHA: `v002-artifact-sha256.txt`
RESTORE_V001_BUILD_RC: 0
RESTORE_LOG: `restore-v001-runner.log`

V002 build artifacts:

- Parent library: `3ad998adbb7a126f15b7da0a8f8e0a816ca719d92a4dc42aba97d7832ffa4db4`
- Candidate library: `0ef46469d226225c47ca4c33286b505b48444175095572bc3abf0e8d49c30126`
- Existing paired runner: `9d1274ae18dc910251ee4c6ed140bc3d240827836c6b69cfac5703e4e5a2a100`

Final restored V001 source/artifact identity is recorded by the restore build; the tracked V001 support diff is empty.

## Correctness

Target `rows=16,width=6144,blocks=8,dtype=fp16`: PASS; `bit_differences=0`, `max_abs=0`, `tolerance_failures=0`, `nonfinite=0`.

M=1 control `rows=1,width=6144,blocks=1,dtype=fp16`: PASS; `bit_differences=0`, `max_abs=0`, `tolerance_failures=0`, `nonfinite=0`.

Evidence:

- `correctness-v002-r16-d6144.log`
- `correctness-v002-m1-d6144.log`

The M=1 pass confirms that the new placement does not alter the no-successor single-row path. The target pass confirms the existing `SyncVToMTE2` producer boundary, next-row `SyncMTE2ToV` consumer boundary, and retained `valueFp32Buf_` path remain correct for this probe.

## Local

RUNNER: existing paired runner
DEVICE: 0
SHAPE: rows=16 width=6144 blocks=8 dtype=fp16
WARMUPS: 45
PAIRED_SAMPLES: 21
TIMING: device-event; `aclrtRecordEvent`, stop-event synchronization, `aclrtEventElapsedTime`
ORDER: alternating PC/CP

Raw output: `local-v002-r16-d6144-w45.tsv`
Runner log: `local-v002-r16-d6144-w45.log`

PARENT_MEDIAN_DEVICE_US: 17.139999
CANDIDATE_MEDIAN_DEVICE_US: 18.400000
MEDIAN_OF_PAIRED_DELTA_DEVICE_US: +0.300000
MEDIAN_OF_PAIRED_DELTA_PERCENT: +4.1436%
PARENT_P05_P95_US: 7.360000 / 25.380000
CANDIDATE_P05_P95_US: 7.500000 / 26.820000
DIRECTION_COUNT: candidate faster 9/21; candidate slower 12/21

## Load and authenticity

Load snapshots: `local-v002-load-pre.txt` and `local-v002-load-post.txt`.

Target device 0 was polluted during the only local run:

- pre: `w4r07_runner` PID 87328 (112 MB) and `VLLMEngineCor` PID 196478 (8075 MB); HBM 11645/65536 MB; AICore 0%
- post: `w4r07_runner` PID 87328 (112 MB) and `VLLMEngineCor` PID 196478 (8073 MB); HBM 11798/65536 MB; AICore 0%

Other devices also carried VLLM/python processes. The V002 local number is retained as raw evidence only; it is not a clean causal performance result and does not advance `LOCAL_BEST`.

LOCAL_VERDICT: LOCAL_NO_PROMOTION / MEASUREMENT_CONTAMINATED
CURRENT_LOCAL_BEST: NONE
V001_EVIDENCE: preserved; no V001 raw was overwritten

## Next action

H1 is implemented and correctness-safe, but this single local run cannot accept or reject its performance mechanism because target-device pollution was present. Planning review must decide whether a clean H1 measurement is warranted. No V003 or additional test is created by this revision.

## server3 lease check

One read-only connection/device/lease check was attempted after the polluted local result. The existing `cann-server3` hostname could not be resolved, so no remote identity, NPU/HBM snapshot, or lease authorization was obtained. The clean paired measurement was not run, and this V002 remains `LOCAL_NO_PROMOTION / MEASUREMENT_CONTAMINATED`. No source, Runner, ABI, timing boundary, shape, or V002 raw data changed.

Evidence: `server3-lease-check.log`

## Current-host doctor

The current Worktree host is `hwnput3` and exposes eight NPUs. Devices 2 and 3 are idle in the `npu-smi info` snapshot with approximately 62 GB free HBM each, but no current R1/V002 lease file or authorization variable is visible. Other devices are occupied, and the installed `npu-smi` rejected the attempted singular `usage` and `proc` type probes with rc=215; its successful `info` output already includes HBM and process status. Because an idle device without a verified lease is not a safe performance target, no paired measurement was run.

Evidence: `server3-doctor-20261009.log`
