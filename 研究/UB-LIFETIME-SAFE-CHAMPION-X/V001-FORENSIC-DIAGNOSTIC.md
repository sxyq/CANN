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
correctness harness. No `.asc`, `.cpp`, `.h`, `.hpp`, CMake, runner, or host
wrapper source was modified. No timing or Online action was performed.

`FINAL_CLASSIFICATION: UNRESOLVED`

The classification is deliberately conservative. The preserved exact V001
binary has a reproducible-looking FP32 failure record, but the existing
harness cannot run the parent without replacing its hard-coded candidate
include, and it does not print the requested first-mismatch/value diagnostics.
Therefore the evidence cannot distinguish a candidate-induced shared-path
effect from a parent/harness effect with proof.

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

## B. Parent same-binary correctness

The existing correctness runner is source-bound by
`support/correctness/correctness_runner.asc`:

```text
#include "../../submission.asc"
#include "correctness_runner.cpp"
```

There is no preserved parent executable in the V001 evidence directory. The
existing runner is hard-bound to V001 by the two includes shown above. Running
the parent would require replacing that include or creating a second runner,
which is forbidden by this task. Therefore:

```text
PARENT_SAME_BINARY_RESULT=MISSING_PARENT_RUN
PARENT_PASS_FAIL=UNAVAILABLE
PARENT_NONDETERMINISM=UNAVAILABLE
```

This is a precise harness/evidence blocker, not a guessed parent result.

## C. Exact V001 rebuild and same-case correctness

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

## D. Mismatch-pattern extraction limits

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

## E. Final decision

The static lifetime ordering for the intended FP16 alias is valid, and the
diff audit proves the declared source scope. However, the only runtime failure
is in the separately dispatched wide-FP32 path, the exact parent baseline
cannot be run with the existing source-bound harness, and the runner omits
the per-element diagnostics needed to identify the corruption pattern.

`FINAL_CLASSIFICATION: UNRESOLVED`

Missing evidence that would be required to distinguish the remaining classes:

- a parent `R31B V011` executable/run using the same runner, device, and
  `fp32-wide-16384` case, repeated enough to establish its baseline;
- a permitted diagnostic runner that reports first mismatch, values,
  `invRms`, special-value locations, and padding/tile distribution.

No V002, source fix, timing, Online submission, shared-ledger update, or
route-lifecycle decision was made.
