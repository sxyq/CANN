# Public R1 V001 backend validation through R2 w5ctl (corrected)

## Authoritative correction and final status

The initial WIDE-X-FRESH4/V001 selection in this receipt was the wrong public
input for the requested W5-R01 validation. Its Parent was `BUILD-FIX-001`, and
its Build/Correctness/Local material is retained as a failed attempt only; it
is **NOT_ACCEPTED_AS_W5_R01_BACKEND_VALIDATION** and is not a Register-Spill
result.

The formal validation below uses only the requested public Git commit
`d2412ab5cb2d091679947c65d90d45912541b1a5`, materialized under this R2
worktree without reading or modifying an R1 private worktree:

```text
FORMAL_PUBLIC_ROUTE=CROSSROW-EPI-PROLOGUE-X
FORMAL_PUBLIC_REVISION=V001
FORMAL_PUBLIC_PARENT=W3 CROSSROW-FULL-PIPELINE-CHAMPION-X V012
FORMAL_PARENT_SOURCE_SHA256=6075d390df86bb56f7db4d025f11c244791f9d8c722cb4091898fa01940cf794
FORMAL_CANDIDATE_SOURCE_SHA256=3698c229dfb6a4b336e7ee0506eb1ceaf898c1c37c937c0af7d3eef71fbc685d
FORMAL_RUNNER_SOURCE_SHA256=6096c0bd0fa6f26384a2967adb7222d8d4712a781b855cc4579152c3e3fd502f
FORMAL_INPUT_ROOT=研究/REGISTER-SPILL-ELIMINATION-X/public-crossrow-v001
R2_REGISTER_SPILL_STATUS=FEASIBILITY_UNPROVEN
R2_KERNEL_CANDIDATE=NONE
R2_V001_CREATED=NO
```

The exact public support runner was built through the R2-owned `w5ctl` path.
Build returned `BACKEND_RC=0` and `W5CTL_RC=0`; the paired runner artifact
SHA-256 was
`70fce83a4c70cfd16de3433b2c6beb72a9c0a5b5b2600cd76da3b39550ac75c9`.
Correctness ran once on device 3 for rows=16, width=6144, blocks=8, fp16 and
returned `BACKEND_RC=0`, `W5CTL_RC=0`, `bit_differences=0`, `max_abs=0`,
`tolerance_failures=0`, and `nonfinite=0`. Source and artifact identities
matched before and after delegation.

The one permitted fresh Crossrow Local run used the existing paired runner
with 45 warmups and 21 PC/CP samples. The runner returned `RC=0`; w5ctl
preserved and parsed the unmodified 21-row raw TSV as 42 device-event values:

```text
PARENT_MEDIAN_US=20.800000
CANDIDATE_MEDIAN_US=26.500000
MEDIAN_RATIO_DELTA=0.274038
PAIRED_MEDIAN_DELTA=5.400000
PAIRED_MEDIAN_PERCENT_DELTA=0.255924
RAW_PAIR_COUNT=21
RAW_SHA256=017b507496bce7e4801243242ce90770a85dbe20014f7b4ec881fc37bace360e
MEASUREMENT_QUALITY=UNCLASSIFIED
QUALITY_CLASSIFICATION=UNCLASSIFIED
NOISE_STATUS=NOISY
PROMOTION_ELIGIBLE=NO
W5CTL_LOCAL_RC=66
```

The raw runner schema has no per-sample quality or contamination provenance;
the numeric result is therefore not accepted or promotable. `NOISY` reflects
the observed max CV of 0.447037 versus the 0.20 threshold, while
`UNCLASSIFIED` independently prevents promotion. No quality value was
synthesized and the raw file was not filtered or rewritten.

The preflight and postflight snapshots both observed device 3 at HBM usage
5% with no device-3 process in the `npu-smi` snapshot. The shared lease file
`/home/data4t2/lelinfeng/cann/调度/服务器设备使用.tsv` had mtime
`2026-09-28 19:39:58 UTC`; its device-3 state is therefore
`LAST_KNOWN_NO_ACTIVE_LEASE`, not live authorization. Current lease authority
is `UNKNOWN_STALE_LOG`; no live lease was confirmed.

The w5ctl `record` capability verified source, artifact, and raw evidence
hashes but returned `NOT_RUN`/RC=78 because `RECORD_BACKEND` is not configured.
The `report` capability returned `NOT_RUN`/RC=78 because no checkpoint exists.
Therefore `W5CTL_E2E=INCOMPLETE`; this receipt is historical backend evidence
only and must not be counted as an R2 Kernel score or an Online result.

The parser/self-test change is route-owned. Final self-test command was
`python3 -B 研究/REGISTER-SPILL-ELIMINATION-X/tests/test_w5ctl.py` and passed
8/8, including the `.sh` dispatch return-code test and the Crossrow 21-pair
TSV test that verifies statistics, unclassified status, no promotion, and
unchanged raw bytes. The final test log is
`研究/REGISTER-SPILL-ELIMINATION-X/w5ctl-crossrow-parser-selftest-retry1.log`.

Evidence paths retained under this R2 route include:

```text
BUILD_PASS=研究/REGISTER-SPILL-ELIMINATION-X/crossrow-public-build-w5ctl-retry3.log
BUILD_BACKEND_PASS=研究/REGISTER-SPILL-ELIMINATION-X/crossrow-public-build-backend-retry3.log
BUILD_FAILED_ATTEMPTS=研究/REGISTER-SPILL-ELIMINATION-X/crossrow-public-build-w5ctl.log;crossrow-public-build-w5ctl-retry1.log;crossrow-public-build-w5ctl-retry2.log
CORRECTNESS_PASS=研究/REGISTER-SPILL-ELIMINATION-X/crossrow-public-correctness-w5ctl-retry1.stdout.log
CORRECTNESS_BACKEND_PASS=研究/REGISTER-SPILL-ELIMINATION-X/crossrow-public-correctness-w5ctl-retry1.log
LOCAL_PREFLIGHT=研究/REGISTER-SPILL-ELIMINATION-X/crossrow-public-local-device3-preflight.log
LOCAL_RAW=研究/REGISTER-SPILL-ELIMINATION-X/crossrow-public-paired-local-device3.tsv
LOCAL_BACKEND=研究/REGISTER-SPILL-ELIMINATION-X/crossrow-public-paired-local-device3-backend.log
LOCAL_W5CTL=研究/REGISTER-SPILL-ELIMINATION-X/crossrow-public-paired-local-device3-w5ctl.log
LOCAL_POSTFLIGHT=研究/REGISTER-SPILL-ELIMINATION-X/crossrow-public-local-device3-postflight.log
RECORD_STATUS=研究/REGISTER-SPILL-ELIMINATION-X/crossrow-public-record-report-status.log
REPORT_STATUS=研究/REGISTER-SPILL-ELIMINATION-X/crossrow-public-report-status.log
SELFTEST_PASS=研究/REGISTER-SPILL-ELIMINATION-X/w5ctl-crossrow-parser-selftest-retry1.log
```

Build failures and the first malformed correctness invocation are also retained
in separate `crossrow-public-*` attempt logs; no failed log was overwritten.

AGENT_ID=W5-R02-REGISTER-SPILL-ELIMINATION-X
WORKTREE=/home/data4t2/lelinfeng/cann-w5-r02-register
BRANCH=research/w5-r02-register-spill
R2_REVISION=NONE
CURRENT_STAGE=PUBLIC_CROSSROW_VALIDATION_COMPLETE
PUBLIC_INPUT_COMMIT=d2412ab5cb2d091679947c65d90d45912541b1a5
RECEIPT_TIME_UTC=2026-10-10

## Misselected WIDE-X attempt identity (retained only)

Inputs were read from the public Git object above and the matching tracked
files in this R2 worktree. No R1 private worktree was accessed, no source was
copied, and no R2 Register-Spill Candidate/V001 was created. The R1 V001
direct Parent is `BUILD-FIX-001`, not a top Champion donor.

```text
R1_ROUTE=WIDE-X-FRESH4
R1_REVISION=V001
R1_PARENT=BUILD-FIX-001
PARENT_SOURCE=本地实验/WIDE-X-FRESH4/BUILD-FIX-001/submission.asc
PARENT_SOURCE_SHA256=5d0ee01165e46a281cbb7d1605feba3605ad59883845400a063e97704f27f2be
CANDIDATE_SOURCE=本地实验/WIDE-X-FRESH4/V001/submission.asc
CANDIDATE_SOURCE_SHA256=f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895
BUILD_BACKEND=本地实验/WIDE-X-FRESH4/V001/support/build_server3.sh
RUNNER_SOURCE=本地实验/WIDE-X-FRESH4/V001/support/runner_main.cpp
RUNNER_SOURCE_SHA256=2c2904615ee6445f5b49c7d862daf453ee8837518ebf6e0732d854e97625b84e
```

`submission.asc`, `source-meta.json`, `submission.sha256`, `diff.patch`,
`support/build_server3.sh`, `support/CMakeLists.txt`, `support/runner_main.cpp`,
`support/README.md`, and `support/runner-validation.md` were checked against
the exact public commit. Current tracked content matched each public Git blob.

## Misselected WIDE-X Build attempt (not accepted)

The public build script is mode `100644`, so the first w5ctl call returned
`RC=78 BACKEND_NOT_EXECUTABLE` before delegation. R2-only w5ctl was then
adapted to allow an explicitly selected non-executable `.sh`/`.bash` file via
the system `/bin/bash`; no R1 file or permission was changed.

Exact successful command:

```bash
timeout 240 python3 -B 研究/REGISTER-SPILL-ELIMINATION-X/w5ctl build \
  --repo-root /home/data4t2/lelinfeng/cann-w5-r02-register \
  --expected-branch research/w5-r02-register-spill \
  --route W5-R02-REGISTER-SPILL-ELIMINATION-X \
  --route-root /home/data4t2/lelinfeng/cann-w5-r02-register/研究/REGISTER-SPILL-ELIMINATION-X \
  --source /home/data4t2/lelinfeng/cann-w5-r02-register/本地实验/WIDE-X-FRESH4/V001/submission.asc \
  --artifact /home/data4t2/lelinfeng/cann-w5-r02-register/本地实验/WIDE-X-FRESH4/V001/support/build/wide_x_fresh4_unified_runner \
  --expected-source-sha f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895 \
  --backend /home/data4t2/lelinfeng/cann-w5-r02-register/本地实验/WIDE-X-FRESH4/V001/support/build_server3.sh \
  --backend-workdir /home/data4t2/lelinfeng/cann-w5-r02-register/本地实验/WIDE-X-FRESH4/V001/support \
  --log /home/data4t2/lelinfeng/cann-w5-r02-register/研究/REGISTER-SPILL-ELIMINATION-X/r1-public-build-w5ctl.log
```

```text
W5CTL_BACKEND_COMMAND=/bin/bash <public build_server3.sh>
BACKEND_RC=0
W5CTL_RC=0
SOURCE_SHA256_BEFORE= f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895
SOURCE_SHA256_AFTER=  f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895
RUNNER_ARTIFACT=wide_x_fresh4_unified_runner
RUNNER_ARTIFACT_SHA256=5c290b9294ae8abbfab32cb1d0f9ee6c277d610ca49d6701494bdac0c2c3b94e
PARENT_LIBRARY_SHA256=9f03f15232d984577e559a64e90ec4e550fbef9392d6edb7eedd47ddc530071e
CANDIDATE_LIBRARY_SHA256=c30be731387e0672359f47e748d65f67b7ce0e3f294dbc4f0fe986934b0890ab
IDENTITY_STATUS=PASS
LDD_UNRESOLVED_DEPENDENCIES=NONE
```

The build log is `研究/REGISTER-SPILL-ELIMINATION-X/r1-public-build-w5ctl.log`,
SHA-256 `e9772acddf70b12083ed6a6032dfac31795c4504ea4d042cf57cf60050d2b8e1`.

## Misselected WIDE-X Correctness attempt (not accepted)

The public runner's `correctness-only` mode was invoked for Parent and
Candidate at widths 2048/16384/32768 and dtypes FP32/FP16/BF16 on device 3.
The first attempt returned `RC=127` before ACL initialization because
`libruntime.so` was absent from the child `LD_LIBRARY_PATH`. That failure is
retained as `r1-correctness-parent-fp32-2048.log`. With the CANN toolkit and
driver library paths below, the same case passed, followed by the other 17
cases:

```text
ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
ASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
LD_LIBRARY_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common
```

The w5ctl invocation shape was:

```bash
env <CANN environment above> python3 -B 研究/REGISTER-SPILL-ELIMINATION-X/w5ctl correctness \
  --repo-root /home/data4t2/lelinfeng/cann-w5-r02-register \
  --expected-branch research/w5-r02-register-spill \
  --route W5-R02-REGISTER-SPILL-ELIMINATION-X \
  --route-root /home/data4t2/lelinfeng/cann-w5-r02-register/研究/REGISTER-SPILL-ELIMINATION-X \
  --source /home/data4t2/lelinfeng/cann-w5-r02-register/本地实验/WIDE-X-FRESH4/V001/submission.asc \
  --artifact /home/data4t2/lelinfeng/cann-w5-r02-register/本地实验/WIDE-X-FRESH4/V001/support/build/wide_x_fresh4_unified_runner \
  --expected-source-sha f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895 \
  --expected-artifact-sha 5c290b9294ae8abbfab32cb1d0f9ee6c277d610ca49d6701494bdac0c2c3b94e \
  --backend /home/data4t2/lelinfeng/cann-w5-r02-register/本地实验/WIDE-X-FRESH4/V001/support/build/wide_x_fresh4_unified_runner \
  --backend-workdir /home/data4t2/lelinfeng/cann-w5-r02-register/本地实验/WIDE-X-FRESH4/V001/support/build \
  --backend-arg 3 --backend-arg <WIDTH> --backend-arg <DTYPE> \
  --backend-arg correctness-only --backend-arg <parent|candidate> \
  --backend-arg 0 --backend-arg 0 --backend-arg 1 \
  --backend-arg <R2 research output TSV> --log <R2 research output log>
```

`<WIDTH>/<DTYPE>/<side>` covered all 18 combinations above; each backend and
w5ctl invocation returned `0`, `mismatches=0`, `nonfinite=0`. All 18 raw
correctness TSV/log pairs are retained under
`研究/REGISTER-SPILL-ELIMINATION-X/r1-correctness-*`.

## Misselected WIDE-X Paired Local attempt (not accepted)

Device 3 snapshots before and after showed HBM usage 5% of 65,536 MB, AICore
0%, AIVector 0%; `npu-smi info` showed no running process on device 3. These
are load context, not a stop condition.

Exact command (FP32, width 2048, device 3, 45 warmups, 21 samples/block,
4 paired blocks):

```bash
env ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 \
  ASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 \
  LD_LIBRARY_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common \
  timeout 120 python3 -B 研究/REGISTER-SPILL-ELIMINATION-X/w5ctl local \
  --repo-root /home/data4t2/lelinfeng/cann-w5-r02-register \
  --expected-branch research/w5-r02-register-spill \
  --route W5-R02-REGISTER-SPILL-ELIMINATION-X \
  --route-root /home/data4t2/lelinfeng/cann-w5-r02-register/研究/REGISTER-SPILL-ELIMINATION-X \
  --source /home/data4t2/lelinfeng/cann-w5-r02-register/本地实验/WIDE-X-FRESH4/V001/submission.asc \
  --artifact /home/data4t2/lelinfeng/cann-w5-r02-register/本地实验/WIDE-X-FRESH4/V001/support/build/wide_x_fresh4_unified_runner \
  --expected-source-sha f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895 \
  --expected-artifact-sha 5c290b9294ae8abbfab32cb1d0f9ee6c277d610ca49d6701494bdac0c2c3b94e \
  --raw /home/data4t2/lelinfeng/cann-w5-r02-register/研究/REGISTER-SPILL-ELIMINATION-X/r1-public-paired-fp32-2048.tsv \
  --backend /home/data4t2/lelinfeng/cann-w5-r02-register/本地实验/WIDE-X-FRESH4/V001/support/build/wide_x_fresh4_unified_runner \
  --backend-workdir /home/data4t2/lelinfeng/cann-w5-r02-register/本地实验/WIDE-X-FRESH4/V001/support/build \
  --backend-arg 3 --backend-arg 2048 --backend-arg fp32 \
  --backend-arg paired --backend-arg - --backend-arg 45 \
  --backend-arg 21 --backend-arg 4 \
  --backend-arg /home/data4t2/lelinfeng/cann-w5-r02-register/研究/REGISTER-SPILL-ELIMINATION-X/r1-public-paired-fp32-2048.tsv \
  --log /home/data4t2/lelinfeng/cann-w5-r02-register/研究/REGISTER-SPILL-ELIMINATION-X/r1-public-paired-fp32-2048.log
```

```text
RUNNER_BACKEND_RC=0
W5CTL_RC=66
RAW_SAMPLE_ROWS=168 (4 blocks * 21 samples * Parent/Candidate)
RAW_SHA256=c655f24409659cb165e3a1df74f6e9966db732cf61be767c9f0d8430cb934cfc
W5CTL_RAW_SAMPLE_COUNT=0 (parser rejected row 1; not the physical file row count)
MEASUREMENT_QUALITY=RAW_PARSE_ERROR
NOISE_STATUS=UNKNOWN
LOCAL_STATISTICS_REASON=RAW_ROW_INVALID:1
MEDIAN_PARENT_US=NONE
MEDIAN_CANDIDATE_US=NONE
MEDIAN_RATIO_DELTA=NONE
PAIRED_MEDIAN_DELTA=NONE
PAIRED_MEDIAN_PERCENT_DELTA=NONE
CONTAMINATION=NOT_CLASSIFIED; raw format has no explicit quality/contamination fields
```

The R1 runner writes provenance comment lines before its header, then columns
`block/sample/order/side/source_sha256/device/width/dtype_id/device_event_us/wall_us`.
The current w5ctl parser expects a row-oriented `pair`, `variant`,
`latency_us`/supported alias and explicit `quality`; it interprets the leading
comment as its header and rejects row 1. The raw is preserved unchanged; no
quality or pair fields were synthesized. Thus the real paired runner executed,
but the w5ctl Local statistics/acceptance stage did not pass.

## Misselected WIDE-X attempt result (retained; not accepted as W5-R01)

```text
BUILD=PASS; backend RC=0
CORRECTNESS=PASS; 18/18 cases, all mismatch/nonfinite counts zero
LOCAL_BACKEND=EXECUTED; runner RC=0; w5ctl statistics RC=66 / REJECTED
LOCAL_SCORE=NONE; accepted Local verdict=NONE
RECORD_BACKEND=NOT_CONFIGURED; not run
DEDUP=NOT_RUN; no R2 candidate patch exists
W5CTL_E2E=INCOMPLETE; NOT_PASS
R2_REGISTER_SPILL=FEASIBILITY_UNPROVEN
R2_CANDIDATE=NONE; V001_CREATED=NO
ONLINE=NOT_SUBMITTED
NEXT_ACTION=adapt w5ctl parser/provenance contract to the published runner format
              without fabricating quality; then re-evaluate the retained raw
BLOCKER=raw schema lacks required explicit pair/quality/contamination fields;
         Record backend remains unconfigured
GIT_COMMIT=SEE_COMMIT_CONTAINING_THIS_RECEIPT
```
