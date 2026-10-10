# CROSSROW-EPI-PROLOGUE-X V002 Result

AGENT_ID: W5-R01 CROSSROW-EPI-PROLOGUE-X
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V002
STATUS: NEEDS_ONE_MORE_LOCAL_HIGH_JITTER_NO_SAME_BINARY_PRECHECK

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

## Authorized lease gate

The current user instruction requires a real authorized non-conflicting lease before measuring. The existing Route-owned evidence exposes no current R1/V002 lease file or authorization variable. Devices 2 and 3 meet the numeric HBM threshold and show no running NPU process, but they cannot be claimed without authorization. No shared scheduler/tooling or other Route was inspected, no V002 Runner invocation was made, and the prior polluted raw remains the only Local evidence. V002 stays `LOCAL_NO_PROMOTION / MEASUREMENT_CONTAMINATED`; no V003 was created.

Evidence: `lease-blocker-20261009.log`

## R1/Zeno current-context doctor

The current execution context is the NPU host `hwnput3` and `npu-smi info` sees eight devices. NPU 0-3 have no running NPU processes and more than 62 GB free HBM, while NPU 4-7 are occupied. No current R1/V002 lease or authorization is visible. The existing Runner is present, but its current SHA256 (`5210262b...`) differs from the V002 execution artifact SHA256 (`9d1274ae...`), so exact V002 identity was not claimed. No Runner invocation or new measurement was made.

Evidence: `zeno-doctor-20261009.log`

## Source identity rebuild and correctness

The V002 source was rebuilt from the recorded `Parent.asc` and `Candidate.asc` through the existing V001 CMake/adapter entry. New artifact hashes and the exact environment/build contract are recorded in `V002-SOURCE-IDENTITY-BUILD.md`; the prior V002 manifest was not reused as a substitute. The target and M=1 correctness probes both passed with zero differences. The temporary V002 adapter/metadata retarget was restored and the existing V001 support entry rebuilt successfully.

No authorized R1/V002 lease or owner was available, so Local was not run. V002 remains `LOCAL_NO_PROMOTION / MEASUREMENT_CONTAMINATED`; no V003 was created.

## 2026-10-10 HBM-qualified V002 recheck

The current execution rule permits Local when the helper reports `FREE_HBM_MB > 100`; external processes are recorded as load quality and are not a stop condition. NPU3 was selected because `free_hbm_mb 3` reported `48496 MB` before and after the run. The existing V001 paired backend was temporarily rebound to the route-owned V002 `Parent.asc` and `Candidate.asc`, rebuilt cleanly, and restored/rebuilt as V001 after the run. No new Runner or Revision was created.

V002 source identity remains Parent `3698c229dfb6a4b336e7ee0506eb1ceaf898c1c37c937c0af7d3eef71fbc685d` and Candidate `426489bfdc63441f2fd179fdff176875b24d815e6e0420343aeed0b0dab7933f`. The new V002-bound artifacts are Parent `3ad998adbb7a126f15b7da0a8f8e0a816ca719d92a4dc42aba97d7832ffa4db4`, Candidate `0ef46469d226225c47ca4c33286b505b48444175095572bc3abf0e8d49c30126`, and paired Runner `9ba784c8748af1c33eeb92cb651203d82a7ffc224294c6ccb8e69b95f8db827f`. Full identity and restore hashes are in `v002-artifact-sha256-20261010.txt`.

Build PASS: `rebuild-v002-local-20261010.log`. Target correctness and M=1 correctness both PASS with zero differences: `correctness-v002-clean-20261010-r16-d6144.log` and `correctness-v002-clean-20261010-m1-d6144.log`.

One Local was run with 45 warmups, 21 device-event paired samples, and alternating PC/CP at `rows=16,width=6144,blocks=8,fp16`. Raw: `local-v002-r16-d6144-w45-20261010.tsv`. Parent median was `17.580001 us`; Candidate median was `17.920000 us`; median-ratio was `+1.934010%`; paired-percent was `+1.592708%`; median paired delta was `+0.279998 us`; Candidate was faster in `10/21` samples and slower in `11/21`.

NPU3 had external PID `1859841` (`zhangka+`, `python3.11`, `14238 MB`) in both pre/post snapshots, with target AICore about 11%/13%; other devices also had external processes. Load quality is `CONTAMINATED_EXTERNAL_PROCESS`. The raw result is retained, but this Local is `LOCAL_REJECTED / MEASUREMENT_CONTAMINATED`, does not advance `LOCAL_BEST`, and requires `NEEDS_ONE_MORE_LOCAL` only if Planning requests a clean causal repeat. Evidence: `V002-LOCAL-RECHECK-20261010.md`.

## 2026-10-10 13:45 UTC V002 paired Local identity/jitter recheck

This later run supersedes the earlier V002 Local verdict for current status only; all earlier raw and conclusions remain unchanged. It used the same route-owned V002 sources and the existing V001 CMake/adapter/paired-runner path. Parent and Candidate were both loaded into one paired-runner executable and interleaved within the same process (`SAME_BINARY_PROCESS=YES`). `SAME_BINARY_QUALIFICATION=NOT_RUN`: current published Local rules do not require a separate Parent-only precheck, and this existing runner has no qualification mode; this is recorded as not run, not as a pass.

SOURCE_SHA256: Parent `3698c229dfb6a4b336e7ee0506eb1ceaf898c1c37c937c0af7d3eef71fbc685d`; Candidate `426489bfdc63441f2fd179fdff176875b24d815e6e0420343aeed0b0dab7933f`; OFAT diff `e8e656afd7de4f3c565c5ffe7e84758e44ffb4537ffea1fcbf211e4387c9c9ad`.
V002_ARTIFACTS: Parent `.so` `3ad998adbb7a126f15b7da0a8f8e0a816ca719d92a4dc42aba97d7832ffa4db4`; Candidate `.so` `0ef46469d226225c47ca4c33286b505b48444175095572bc3abf0e8d49c30126`; paired runner `7c3cbde4f33a1c8be8b5637b34a78251f8adfb0141b27527d6dafc49abffd91e`; runner source `093ad39377bf4615d1827ba967ab8f9a6910395d9a7cfd01c513865c5f489526`; ABI `0897a04ba82bda39d53e77d548966eb54a6b7ec16c17387dccf26d86cbf577d2`.

BUILD_COMMAND: `source /usr/local/Ascend/ascend-toolkit/set_env.sh; export CPLUS_INCLUDE_PATH and CPATH to HCC C++ root/target/backward; cmake --build 本地实验/CROSSROW-EPI-PROLOGUE-X/V001/support/build --clean-first --parallel 1`; BUILD_RC: 0; log `rebuild-v002-final-20261010T1340.log`.
CORRECTNESS_COMMAND: `source /usr/local/Ascend/ascend-toolkit/set_env.sh; crossrow_v001_paired_runner --mode correctness --device 3 --rows 16 --width 6144 --blocks 8 --dtype fp16`; RC: 0; PASS, zero bit differences; log `correctness-v002-clean-20261010T1344-r16-d6144.log`.
LOCAL_COMMAND: `source /usr/local/Ascend/ascend-toolkit/set_env.sh; crossrow_v001_paired_runner --mode local --device 3 --rows 16 --width 6144 --blocks 8 --dtype fp16 --warmups 45 --repeats 21 --output local-v002-r16-d6144-w45-20261010T1345.tsv`; RC: 0. Device-event timing, alternating PC/CP, 45 warmups, 21 pairs. Raw has 21 consecutive sample IDs and correct alternating order; it is retained in full, including the `50.159998 us` Candidate sample. Raw SHA256: `0124ff6d57981c58bb74be86937a05c7f46813e9797a069a44047da052bb3f4d`.

PARENT_MEDIAN_DEVICE_US: 21.340000
CANDIDATE_MEDIAN_DEVICE_US: 19.320000
MEDIAN_RATIO_FORMULA: `100 * (median(C) - median(P)) / median(P)`
MEDIAN_RATIO_PERCENT: -9.465792%
MEDIAN_OF_PAIRED_PERCENT_FORMULA: `median_i(100 * (C_i - P_i) / P_i)`
MEDIAN_OF_PAIRED_PERCENT: -3.308124%
MEDIAN_PAIRED_DELTA_US: -0.699999
PARENT_DEVICE_JITTER: mean 20.800952 us; sample stdev 5.395474 us; CV 25.939%; MAD 2.460000 us; min/p10/p90/max 8.020000/14.500000/26.219999/29.400000 us.
CANDIDATE_DEVICE_JITTER: mean 19.150476 us; sample stdev 9.485986 us; CV 49.534%; MAD 2.960000 us; min/p10/p90/max 7.600000/8.080000/27.200000/50.159998 us.
PAIR_DIRECTION: raw delta signs give Candidate faster 13/21, slower 8/21, ties 0. A stated 14/21 count is not reproduced by the 21 raw rows.
WALL_MEDIANS: Parent 74.868000 us; Candidate 74.549000 us. One pair had Candidate wall 112.658000 us alongside 50.159998 device us.

RESOURCE/LOAD: device 3 on `hwnput3`; FREE_HBM 62259 MB pre/post (HBM usage 5%, under 100%); no process on target NPU3 in either full `npu-smi info` snapshot; AICore 0% pre/post. External processes were present on other NPUs and recorded, not used to deny measurement. The checked shared lease table had no active device-3 row; a separate unresolved metadata row referred to device 6 only. Lease/load records are observational for this run.
LOAD_QUALITY: TARGET_NPU3_NO_PROCESS; OTHER_NPU_ACTIVITY_RECORDED; HIGH_WITHIN_RUN_JITTER.
LOCAL_VERDICT: NEEDS_ONE_MORE_LOCAL / MEASUREMENT_QUALIFICATION_INSUFFICIENT_HIGH_JITTER.
LOCAL_BEST: unchanged; no promotion.
RESTORE: V001 support sources restored with empty tracked diff; corrected restore clean build PASS. The initial restore command typo (wrong path character, RC=1) is retained in `restore-v001-backend-20261010T1350.log`; corrected build is `restore-v001-backend-retry-20261010T1351.log`.
EVIDENCE: `V002-LOCAL-RECHECK-20261010T1345.md`, `v002-artifact-sha256-20261010T1342.txt`, and the build/correctness/raw/pre/post logs named above.

Because this run is not qualified for causal acceptance, no next Kernel Revision is opened from its apparent median improvement. A proposed FP16 cached-row MTE3 output-drain direction is excluded: the user-provided cross-route dedup review says it materially overlaps Main-1 R31A V021's delayed output MTE3-to-V wait and approaches STORE-EPILOGUE-X's writeback/drain mechanism (`MECHANISM_OVERLAP=YES`, rejected as duplicate before code). No V003 was created. Next: after committing/pushing this V002 result, perform route-local hypothesis research using only R1 V001/V002 evidence; do not repeat this P/C contrast to mask jitter.
