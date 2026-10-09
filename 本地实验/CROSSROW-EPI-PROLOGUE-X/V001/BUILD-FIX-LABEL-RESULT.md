# V001 Runner Label Build Fix and Measurement

AGENT_ID: W5-R01 CROSSROW-EPI-PROLOGUE-X
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V001
TYPE: BUILD_FIX_METADATA_ONLY
STATUS: IDENTITY_FIX_PASS_MEASUREMENT_CONTAMINATED

## Scope

Only the Parent/Candidate text labels in the existing paired runner copy were corrected. Kernel source, kernel ABI, tensor shapes, allocation, event boundaries, warmups, repeats, and alternating PC/CP order were not changed.

Existing runner entry:

`本地实验/CROSSROW-EPI-PROLOGUE-X/V001/support/build/crossrow_v001_paired_runner`

## Explicit before/after diff

The complete source diff is retained in `RUNNER-LABEL-FIX.diff`.

Before:

- `Parent V001` / `Candidate V012`
- `parent=V001 candidate=V012`

After:

- `Parent W3-CROSSROW-FULL-PIPELINE-V012` / `Candidate W5-R01-CROSSROW-EPI-PROLOGUE-X-V001`
- `parent=W3-CROSSROW-FULL-PIPELINE-V012 candidate=W5-R01-CROSSROW-EPI-PROLOGUE-X-V001`

The four replacements are metadata strings only. `git diff --check` passed.

## Source and artifact identity

RUNNER_SOURCE_SHA_BEFORE: 6096c0bd0fa6f26384a2967adb7222d8d4712a781b855cc4579152c3e3fd502f
RUNNER_SOURCE_SHA_AFTER: e4c46069d21f16b863775461298b452b063430d7ca14fdad3c56ae8c0826d938
PARENT_ASC_SHA256: 6075d390df86bb56f7db4d025f11c244791f9d8c722cb4091898fa01940cf794
CANDIDATE_ASC_SHA256: 3698c229dfb6a4b336e7ee0506eb1ceaf898c1c37c937c0af7d3eef71fbc685d
RUNNER_ABI_SHA256: 0897a04ba82bda39d53e77d548966eb54a6b7ec16c17387dccf26d86cbf577d2

Compiled artifact hashes are retained in `label-fix-artifact-sha256.txt`:

- `libcrossrow_v001_parent.so`: 61eb3391924fabbc7b72231652914df2688ca28d376ed143b5d351042b93e944
- `libcrossrow_v001_candidate.so`: 278dbd1cd0866f30ce25b2ea9a3c5fd5214b641bc8ae78e3de52b6d35c3ff34b
- `crossrow_v001_paired_runner`: 5210262b45c50878c24def38e108386b45bd017086a61a1293ab290d171a4d5a

## Build

BUILD_ENTRY: existing `support/build` CMake entry
BUILD_RC: 0
BUILD_LOG: `compile-v001-label-fix.log`
TARGETS: `crossrow_v001_parent`, `crossrow_v001_candidate`, `crossrow_v001_paired_runner`

## Launch failure retained

The first launch was attempted without the existing CANN runtime environment and stopped before entering the runner:

FAILED_LAUNCH_RC: 127
FAILED_LAUNCH: `libruntime.so: cannot open shared object file`
FAILED_LAUNCH_LOG: `local-v001-r16-d6144-w45-label-fix.log`
FAILED_LAUNCH_RAW: none created

The successful retry sourced `set_env.sh`; it is a runtime-environment retry, not an additional measurement sample.

## Recheck

RUNNER: existing `crossrow_v001_paired_runner`
MODE: local
DEVICE: 0
SHAPE: rows=16 width=6144 blocks=8 dtype=fp16
EFFECTIVE_BLOCKS: 8
ROWS_PER_BLOCK: 2
WARMUPS: 45
PAIRED_SAMPLES: 21
TIMING: device events via `aclrtRecordEvent`, stop-event synchronization, and `aclrtEventElapsedTime`
ORDER: alternating PC/CP
CORRECTNESS: PASS; bit_differences=0 max_abs=0 tolerance_failures=0 nonfinite=0

Corrected-label raw output is retained in `local-v001-r16-d6144-w45-label-fix-retry.tsv`; stdout is retained in `local-v001-r16-d6144-w45-label-fix-retry.log`.

RECHECK_RAW_SHA256: 2a373b609921a89b1eb54c2358c01e1a51135737311f6a56e8efb5f833e8c443
RECHECK_LOG_SHA256: 4c6d2b22361151b06c72a2194ca1eae5776fd96e0ebb68f7a4cb1b958f5cfe7a
BUILD_LOG_SHA256: 9e17e9575ef449bbacc0921a47fa161ec58b3682ca81a73d0f66c723f5891c9b
FAILED_LAUNCH_LOG_SHA256: 0131255f1c9c84779fc92e9447785972f93a8b44a74bbfae428df11056f1e857

PARENT_MEDIAN_DEVICE_US: 20.740001
CANDIDATE_MEDIAN_DEVICE_US: 22.940001
MEDIAN_OF_PAIRED_DELTA_DEVICE_US: +0.940001
MEDIAN_OF_PAIRED_DELTA_PERCENT: +4.2727%
PARENT_P05_P95_US: 7.700000 / 25.280001
CANDIDATE_P05_P95_US: 7.520000 / 32.660000
DIRECTION_COUNT: candidate faster 9/21; candidate slower 12/21

The prior raw files remain unchanged and were not overwritten:

- `local-v001-r16-d6144.tsv`: SHA256 d3becb243132985934b7c6c2774ba0fab68f84a20748abfafb3fcac8c50e09ca
- `local-v001-r16-d6144-w45.tsv`: SHA256 705bcccea7731bf7f3c4316de7e5cbba5b7b962318dd972b85ff4bfc693d4ccf

## Device-load context

Fresh retry snapshots are `local-label-fix-retry-load-pre.txt` and `local-label-fix-retry-load-post.txt`.

The target device 0 was not clean during the successful recheck:

- pre: `rayWorkerDict` PID 4072963, about 7269 MB; HBM 10638/65536 MB; AICore 0%
- post: `rayWorkerDict` PID 4072963, about 7807 MB; HBM 11330/65536 MB; AICore 0%
- devices 4-7 also carried VLLM/python processes

Therefore `+0.940001 us / +4.2727%` is retained as raw diagnostic data only. It is not a clean performance conclusion and does not advance `LOCAL_BEST`.

## Authenticity conclusion

IDENTITY_FIX: PASS. The corrected raw labels now match the CMake adapter binding and the Parent/Candidate source SHAs.
KERNEL_CHANGE: NONE.
RUNNER_FLOW_CHANGE: NONE beyond metadata strings.
PERFORMANCE_AUTHENTICITY: BLOCKED by target-device pollution; no causal performance claim is made.
LOCAL_VERDICT: LOCAL_NO_PROMOTION / MEASUREMENT_BLOCKED_FOR_PERFORMANCE_INTERPRETATION.
V002: NOT_CREATED.

NEXT_EXPECTED_STEP: Planning review decides whether a clean, identity-correct measurement is required; do not create V002 or extend this measurement until that decision.
