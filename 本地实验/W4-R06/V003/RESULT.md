# W4-R06 V003 result

DATE=2026-10-09
ROUTE=W4-R06-MTE2-XRES-ISSUE-X
REVISION=V003
DIRECT_PARENT=W4-R06-V002
PARENT_SOURCE_COMMIT=8b1332d460e4a6f328e3f13c54dc74f4940da3ad
PARENT_SOURCE_SHA256=5f6c3443833080f9e4564beb592209a7e2e5e02d4316e61f30515fc37247aacc
CANDIDATE_PATH=本地实验/W4-R06/V003/Candidate.asc
SOURCE_SHA256=73dd051bae16b434ce08c1923d97bfb7c778dd01d9dac11fec2e873e542febeb

## Technical change

For wide x/residual transfers, use `DataCopy` when the valid byte count, UB
destination address, and actual GM source address are all 32-byte aligned.
Otherwise retain the existing `DataCopyPad` load. This changes the eligible
copy form only; arithmetic, reduction, traversal, buffers, synchronization,
and output stores are unchanged. V003 does not use V002's two-block
`DataCopyPad` form.

The local `Candidate.asc` SHA-256 matches the staged
`R31B-V016-WIDE-TILE-SEED_kernel.asc` used by the paired runner. The runner's
legacy binary labels (`V002` in correctness output and `V016` in paired output)
are not the R06 revision identity. The test harness was derived from the
existing R31B paired runner; `support/remove-device-reset.patch` only removes
its device-reset call and is not part of the Candidate source.

## Compile

TARGET=Ascend 910B3 / CANN 8.5.0.alpha002
HOST=cann-server3
REMOTE_TEMP=/tmp/w4-r06-v003.40dvFV
RUNNER=paired_runner_v016
SOURCE_IDENTITY=remote staged candidate SHA-256 matched SOURCE_SHA256

ATTEMPT_1=FAIL: the CANN compiler plugin did not receive `ASCEND_HOME_PATH`;
the compiler exited before producing the object. Evidence: logs/compile-attempt1.log.
ATTEMPT_2=PASS: the same Candidate compiled and linked successfully.
Evidence: logs/compile-attempt2.log.

## Correctness

RESULT=PASS
DEVICE=3
CASES=4; two rows each; FP16 and BF16 at widths 12288 and 32768
RUNNER_COMMAND=paired_runner_v016 --correctness-only
FAILURES=0 in every case
PARENT_CANDIDATE_EXACT=YES in every case

An earlier launch used a nonexistent version-level environment script and
exited before the runner started. The next launch reached the executable but
the CANN toolchain `libstdc++.so.6` lacked `GLIBCXX_3.4.29`; no Kernel ran in
either failed launch. Correctness passed after sourcing the installed
`aarch64-linux/script/set_env.sh` and preloading the system
`/usr/lib/aarch64-linux-gnu/libstdc++.so.6`. The failed loader output and the
passing run are both preserved in logs/correctness.log.

## Local observation

DEVICE=3, Ascend 910B3
METHOD=45 warmups per binary; 21 alternating Parent/Candidate pairs per case;
device-event timing primary; wall timing diagnostic
RAW_SAMPLES=logs/local.log
CURRENT_LOCAL_BEST=NONE
INTERPRETATION=Numeric observations are retained; spread is high and paired
deltas vary in sign, so no stable improvement is claimed.

| Shape / dtype | Parent median (us) | Candidate median (us) | Paired delta median (us) | Device-time CV Parent / Candidate |
|---|---:|---:|---:|---:|
| 2x12288 FP16 | 14.000 | 11.520 | -2.760 | 1.34536 / 0.77229 |
| 2x32768 FP16 | 20.660 | 19.420 | -0.080 | 0.96417 / 1.01981 |
| 2x12288 BF16 | 12.520 | 14.160 | +1.080 | 0.26270 / 0.22262 |
| 2x32768 BF16 | 23.720 | 22.660 | -3.100 | 0.91959 / 0.83008 |

LOAD_NOTE=Before Local, NPU 3 HBM usage was 59658/65536 MB (about 5878 MB
available), AICore 68%, and VLLMEngineCor PID 1872250 used 56138 MB. After
Local, HBM usage remained 59658/65536 MB and AICore was 67%. The unrelated
process was left unchanged. Snapshots: logs/device-load-pre-local.txt and
logs/device-load-post-local.txt.

## Disposition

REAL_PERFORMANCE_CANDIDATE=YES
COMPILE=PASS (attempt 1 environment failure retained; attempt 2 passed)
CORRECTNESS=PASS (4/4 cases)
LOCAL=NUMERIC_OBSERVATIONS; variable, no stable improvement established
OFFICIAL=NOT_SUBMITTED
PUSH=NO

Evidence files in this revision directory preserve the source, compiler
attempts, correctness output, raw paired samples, load snapshots, and the
harness-only patch.
