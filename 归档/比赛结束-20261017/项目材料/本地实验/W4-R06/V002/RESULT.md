# W4-R06 V002 — same-input two-block MTE2 descriptor

DATE=2026-10-09
ROUTE=W4-R06 MTE2-XRES-ISSUE-X
REVISION=V002
DIRECT_PARENT=R31B-V011
PARENT_SOURCE_COMMIT=b09e00eb351bf376a459fff0e690ea0613221b40
PARENT_SOURCE=归档/历史工作区/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE=本地实验/W4-R06/V002/Candidate.asc
CANDIDATE_SHA256=5f6c3443833080f9e4564beb592209a7e2e5e02d4316e61f30515fc37247aacc

## One change

In the FP16/BF16 ProcessWideLowPrecision pass-1, an x tile or residual tile
whose valid byte count is divisible by 64 is copied using one DataCopyPad
descriptor with blockCount=2, equal blockLen=byteCount/2, and zero GM/UB
stride. Thus each block is at least 32 bytes and 32-byte aligned. Other lengths
use the original one-block Load.

x and residual remain separate source views and separate calls. The x-first
issue order, GM addresses and extents, UB destination/capacity, ping-pong
lifetimes, ready/release events, traversal, arithmetic, reduction and outputs
are unchanged. This does not reuse the residual-first issue-order change from
R5 V010/V021.

## Compile

TARGET=Ascend 910B3 / CANN 8.5.0.alpha002
HOST=cann-server3
REMOTE_TEMP=/tmp/w4-r06-v002.JuHKRa
RUNNER=paired_runner_v016 (existing R31B paired runner, with its Candidate
include pointed to this V002 source)

ATTEMPT_1=FAIL: CANN plugin could not locate the standard C++ vector header
because CPLUS_INCLUDE_PATH was not exported. The captured failure is in
logs/compile-attempt1.log.
ATTEMPT_2=PASS: same Candidate source; exported the C++ 11 and CANN HCC
standard-library include paths through CPLUS_INCLUDE_PATH; forced rebuild and
link completed with exit code 0. Full output: logs/compile-attempt2.log.

## Correctness

RESULT=PASS
COMMAND=paired_runner_v016 --correctness-only
DEVICE=3
CASES=2 rows, FP16/BF16, widths 12288 and 32768
The Parent's ChooseWideFullYRows calculation gives these tile widths:
FP16 D=12288: 4096; FP16 D=32768: 2048; BF16 D=12288: 4096;
BF16 D=32768: 2560. The last case therefore has a 2048-element tail.
All tested tile byte counts, including that tail, are divisible by 64 and
exercise the two-block path. A length that takes the one-block fallback was
not included in this runner matrix. Its inherited case string calls
D=12288 "tail", although that width has no tile tail.
For each case, both R31B-V011 and V002 matched the independent CPU reference
under mixed tolerances, with zero failing elements. Parent and V002 output
buffers were also byte-identical in every case. No reference or input data was
changed.

Runner output: logs/correctness.log. The reused runner's performance-mode
label says V016 because that was its original namespace; the actual Candidate
include and copied source SHA256 are the R06 V002 source above.

## Local performance

Runner: same binary, one ACL process/stream, 45 warmups per kernel, 21
alternating parent/candidate pairs per shape, device-event timing primary.
All raw samples and summaries are preserved in logs/local.log. The runner's
legacy case labels are retained in the raw log; D=12288 is not a tile tail.

| Shape/dtype | Tile width / last tile | Parent samples / median (us) | V002 samples / median (us) | Paired median delta (us) | Paired median vs parent median |
|---|---:|---:|---:|---:|---:|
| 2x12288 FP16 | 4096 / 4096 | 21 / 12.620 | 21 / 8.900 | -0.940 | -7.45% |
| 2x32768 FP16 | 2048 / 2048 | 21 / 23.820 | 21 / 21.600 | +0.400 | +1.68% |
| 2x12288 BF16 | 4096 / 4096 | 21 / 19.260 | 21 / 17.360 | +2.420 | +12.56% |
| 2x32768 BF16 | 2560 / 2048 | 21 / 20.240 | 21 / 20.360 | +0.340 | +1.68% |

The percent column is the paired delta median divided by the parent median.
Sample spread was high: device-event CV ranged from 0.521 to 0.712 for Parent
and 0.260 to 0.597 for V002 across the four cases. Paired deltas change sign
within shapes. These observations do not establish a stable performance gain;
CURRENT_LOCAL_BEST=NONE remains unchanged. The numeric medians and raw
observations are retained, not promoted to a best result.

## Device/load context

DEVICE=3, Ascend 910B3. Pre-correctness HBM usage was 59% of 65536 MB;
pre-local HBM usage was 59%, AICore 43%, AIVector 15%, HBM bandwidth 13%.
Post-local usage was HBM 59%, AICore 42%, AIVector 25%, HBM bandwidth 30%.
The device was active during measurements; this was recorded as context and
other work was not altered. Snapshot: logs/device-load-post-local.txt.

## Disposition

REAL_PERFORMANCE_CANDIDATE=YES
COMPILE=PASS (attempt 1 environment failure retained; attempt 2 passed)
CORRECTNESS=PASS
LOCAL=NUMERIC_OBSERVATIONS; noisy, no stable improvement
LOCAL_SCORE_US=FP16_12288:8.900; FP16_32768:21.600; BF16_12288:17.360; BF16_32768:20.360
LOCAL_DELTA_US=FP16_12288:-0.940; FP16_32768:+0.400; BF16_12288:+2.420; BF16_32768:+0.340
LOCAL_BEST=NONE
OFFICIAL=NOT_SUBMITTED
PUSH=NO

The sole performance source is Candidate.asc. Its parent remains available
from PARENT_SOURCE_COMMIT at PARENT_SOURCE, with the recorded parent SHA256.
The separate runner patch under support/ records harness-only adaptation and
is not part of the Candidate kernel change.
