# UB-LIFETIME-SAFE-CHAMPION-X V001 forensic diagnostic

DATE: `2026-10-02 UTC`
ROUTE: `UB-LIFETIME-SAFE-CHAMPION-X`
WORKTREE: `/home/data4t2/lelinfeng/cann-w2-m2-ub`
BRANCH: `w2/m2/ub-lifetime-safe`
REVISION: `V001` (existing artifact; no V002 created)
DIRECT_PARENT: `R31B V011`
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
CANDIDATE_SOURCE_SHA256: `9b73bb5626b5e98be53faeb91eba024b2589bf0b8b5b46e36c48ec18e009f989`

## Scope and final classification

This is a bounded forensic audit of the preserved V001 source and its existing
correctness evidence. No Candidate `.asc`, `.cpp`, `.h`, or `.hpp` source was
modified. The pre-existing correctness runner was not modified; a separate,
route-local UB-DIAG-2 read-only harness was added under the V001 evidence
directory. No timing or Online action was performed.

`FINAL_CLASSIFICATION: PARENT_INSTABILITY`

UB-DIAG-2 compiled and repeatedly ran exact Parent `R31B V011` and exact
Candidate `V001` sources under the same corrected eight-case protocol. The
Parent itself fails and is non-repeatable on `fp32-wide-16384` in both
independent processes, so it is not a stable baseline for attributing that
failure to V001. The complete metrics and raw logs are in
`本地实验/UB-LIFETIME-SAFE-CHAMPION-X/V001/support/ub-diag-2/`.

## A. Exact V001 diff audit

Compared files:

- Parent: `本地实验/UB-LIFETIME-SAFE-CHAMPION-X/V001/parent-submission.asc`
- Candidate: `本地实验/UB-LIFETIME-SAFE-CHAMPION-X/V001/submission.asc`
- Preserved patch: `本地实验/UB-LIFETIME-SAFE-CHAMPION-X/V001/diff.patch`

The byte-bound parent and candidate hashes match the existing sidecars. The
complete unified diff has exactly three hunks:

1. In the `if constexpr (std::is_same<T, half>::value)` wide initialization
   arm, the candidate removes only
   `pipe_.InitBuffer(outputBuf_, tileElems * sizeof(half));` (parent line 88).
   The float arm (parent lines 69–80) and the non-half/BF16 arm (parent lines
   98–111), including their `outputBuf_` behavior, are unchanged.
2. In `ProcessWideLowPrecision`, the shared local declaration changes from
   `outputBuf_.Get<T>()` to an unbound `LocalTensor<T>` (parent line 3299).
3. The half branch binds `outputLocal` to the indexed retained-y range
   `yStore[batchRow * rowWidth + col]` immediately before `FromFloat`; the
   non-half branch restores `outputLocal = outputBuf_.Get<T>()` before its
   unchanged FP32 work.

The diff contains no changes to:

- `TBuf` member declarations (`xBuf_`, `residualBuf_`, `gammaBuf_`,
  `biasBuf_`, `outputBuf_`, `xFp32Buf_`, `residualFp32Buf_`,
  `valueFp32Buf_`, `reduceFp32Buf_`, `gammaFp32Buf_`, `biasFp32Buf_`);
- FP32 work-buffer allocation or `Get<float>()` use;
- `ProcessWideLowPrecision` pass-1 reduction, `invRms` calculation, or
  common FP32 scratch buffers;
- `FromFloat`, `Mul`, `Add`, `Store`, `DataCopyPad`, valid counts, output
  offsets, tile/row mapping, `PipeBarrier`, `SyncVToMTE2`,
  `SyncVToMTE3`, or `SyncMTE3ToV`.

Thus the source-level scope is limited to the declared FP16-wide storage
transition plus the necessary deferred `outputLocal` binding. The declaration
move is textually shared, but the non-half branch immediately binds the same
`outputBuf_` view and has no changed arithmetic or synchronization. It does
not prove that compiled code is free of a shared-path side effect.

## E. Lifetime and alias re-audit

For the selected FP16 alias, `gammaBuf_` retains the half `y` tile during
pass 1 and pass 2 reads it at `ProcessWideLowPrecision` lines 3314–3318.
The candidate's first write of the output role is the same indexed half range
at line 3321, followed by the unchanged `FromFloat` at line 3322. The strict
source order is therefore:

```text
last retained-y read       :3318
first output-role write    :3322
store issue / source read  :3342-3344
store completion wait      :3345
next retained-y generation :3182 of a later batch
```

The candidate does not alias FP32 storage: `valueFp32Buf_`, `xFp32Buf_`,
`residualFp32Buf_`, and `reduceFp32Buf_` remain separate and their allocation
and use are unchanged. `outputBuf_` remains allocated for the non-half wide
arm and all other paths visible in the source. Static ordering supports the
declared half alias, but cannot account for the observed FP32 failure.

## B. Pre-UB-DIAG-2 parent-run limitation

The existing correctness runner is source-bound by
`support/correctness/correctness_runner.asc`:

```text
#include "../../submission.asc"
#include "correctness_runner.cpp"
```

There was no preserved parent executable in the V001 evidence directory. The
pre-existing runner was hard-bound to V001 by the two includes shown above.
Before UB-DIAG-2, the parent status in that runner was therefore:

```text
PRE_UB_DIAG_PARENT_SAME_BINARY_RESULT=MISSING_PARENT_RUN
PRE_UB_DIAG_PARENT_PASS_FAIL=UNAVAILABLE
PRE_UB_DIAG_PARENT_NONDETERMINISM=UNAVAILABLE
```

This was a precise pre-diagnostic evidence gap, not a guessed parent result. It
was closed by the separate exact-source UB-DIAG-2 harness; it is not a current
blocker.

## C. Pre-UB-DIAG-2 exact V001 rebuild and same-case correctness

After one environment-only failed attempt and one toolkit/plugin include-path
failed attempt, the existing correctness target rebuilt successfully from the
unchanged V001 source. The resulting executable SHA-256 is
`c7b8fc47ee0e54f509499fd5829dcc5744eefcd9e57d4d323a1f51dd57f30849`, matching
the preserved V001 binary identity. Device 7 had `24621/65536` MB HBM used
before the run, i.e. `40915 MB` free; no timing was performed.

Two runs of the exact rebuilt binary on device 7 produced seven identical
non-target PASS lines and the following failing `fp32-wide-16384` lines:

```text
run 1: result=FAIL failures=16342 max_abs=3.32307e+38 max_rel=inf rc=3
run 2: result=FAIL failures=16347 max_abs=inf max_rel=inf rc=3
```

The earlier preserved run on device 4 recorded failures `16337` and
`max_abs=3.2167317e+38`. The changing aggregate failure count/max error across
the same binary/case is `NONDETERMINISTIC` evidence, not a timing result.
Complete command output and resource excerpts are in
`本地实验/UB-LIFETIME-SAFE-CHAMPION-X/V001/support/forensic-execution-20261002.txt`.

## D. Pre-UB-DIAG-2 mismatch-pattern extraction limits

The existing runner prints only one aggregate line after comparing the full
row. The failing case is one full row of 16384 elements (four 4096-element
tiles), not a tail case. From the printed failure count, the derived matched
ratios are:

| run | failures | derived matches | derived matched ratio | max abs | max rel |
|---|---:|---:|---:|---:|---:|
| preserved device-4 run | 16337 | 47 | 0.00286865234375 | 3.2167317e+38 | inf |
| fresh run 1 | 16342 | 42 | 0.0025634765625 | 3.32307e+38 | inf |
| fresh run 2 | 16347 | 37 | 0.00225830078125 | inf | inf |

First mismatch index, per-element output/reference values, reference or actual
`invRms`, mismatch location distribution, NaN/Inf counts/locations, and
padding status are not available from this runner. The second fresh run's
aggregate `max_abs=inf` proves an infinite aggregate difference was observed,
but does not identify the element or whether other special values were
present. No stronger pattern is inferred.

## E. UB-DIAG-2 final decision

The static lifetime ordering for the intended FP16 alias remains valid, and the
diff audit proves the declared source scope. UB-DIAG-2 then built and ran the
exact Parent and Candidate separately with the same diagnostic runner. All
seven FP16/BF16 cases pass identically for both sources, including the FP16
tail case. The only failing case is the unchanged wide-FP32 control, and it
fails in both Parent processes and both Candidate processes with a differing
run-to-run output each time.

`FINAL_CLASSIFICATION: PARENT_INSTABILITY`

The corrected protocol records first mismatch index `0`, actual/reference
outputs, max absolute/relative error, NaN/Inf counts, reference `invRms`,
tail/non-tail status, and repeatability. For the failing FP32 case, Parent
repeatability differs in `12032` and `12544` elements across the two processes;
Candidate differs in `11392` and `13312` elements. Both sources have
`actual_nan=0 actual_inf=0 reference_nan=0 reference_inf=0`; actual `invRms`
is unavailable because the kernel exposes no such output. See the complete
table and raw logs in
`本地实验/UB-LIFETIME-SAFE-CHAMPION-X/V001/support/ub-diag-2/correctness-evidence-20261002.md`.

The old runner's FP32 host-width defect was corrected in UB-DIAG-2 by using
4-byte FP32 storage and reference decoding. The remaining identical Parent
failure family is therefore not classified as `HARNESS_BUG`. Because the
Parent is itself unstable on the failing case, this evidence does not support
`CANDIDATE_BUG` or `SHARED_PATH_SIDE_EFFECT`; the precise classification is
`PARENT_INSTABILITY`, not generic `UNRESOLVED_WITH_EXACT_BLOCKER`.

No V002, source fix, timing, Online submission, shared-ledger update, or
route-lifecycle decision was made.
