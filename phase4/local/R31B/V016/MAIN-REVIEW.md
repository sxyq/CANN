# R31B V016 Main Review

Date: 2026-09-25

## Disposition

No local performance verdict has been formed. Keep V016 unchanged. Do not create V017, promote, reject, or submit online from this evidence. The changed FP16/BF16 correctness domain passes; same-binary remains unrun and timing is waiting for exact-shape qualification and a valid lease. Parent and Candidate both fail FP32 D=16384, and their `ProcessWideFp32FullCacheRows` implementations are byte-for-byte identical. This is not a Candidate regression; treat this exact shape as a shared baseline/reference limitation and exclude it from timing. The underlying fault within the shared kernel/runtime versus runner/reference remains unresolved because no independent oracle was run.

## Source review

- Route: `R31B`
- Revision: `V016`
- Direct Parent: `R31B-V011`
- Parent score: `45.16`
- Parent source SHA-256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate source SHA-256: `9f5c353e65a13a740fe97dc7e6415df032d27560831a3ad142c77592b8208eb5`
- Context: `HISTORICAL_EXPLOIT`
- Hypothesis: initialize only the existing FP16/BF16 wide-row tile at 8192 elements rather than 4096; retain FP32 at 4096 and preserve the UB-budgeted row selection, arithmetic, and synchronization.
- `SINGLE_CHANGE_AUDIT=PASS`. Parent-to-Candidate source comparison confines executable changes to the dtype-specific initial tile size and the assignments needed to preserve the FP32 value. No second performance mechanism was found.

## Evidence

- Candidate SHA matches `source-meta.json`, `local-result.json`, and `correctness-evidence.txt`.
- Existing compile and link records report PASS.
- Targeted FP16/BF16 correctness passed at widths 8192, 12288, 16384, and 32768.
- The FP32 D=16384 control failed for both V011 and V016 under the same runner. The complete `ProcessWideFp32FullCacheRows` function is byte-for-byte identical; the failure counts vary across recorded runs. Classification: common baseline/reference failure, not a V016 regression. Exact attribution between shared FP32 code/runtime and runner/reference is unresolved.
- Correctness-passing shapes eligible to attempt timing qualification: FP32 rows=2,D=8192; FP16 rows=2,D=8192,12288,16384,32768; BF16 rows=2,D=8192,12288,16384,32768. Each shape still requires its own same-binary qualification; this list does not assert timing readiness.
- Four earlier paired observations are `LOAD_CONTAMINATED`; no performance conclusion follows from them.
- The unified paired runner now compiles and links. Parent, Candidate, wrapper, runner sources, CMake input, and runner executable identities are recorded in `route-handoff.md` and `support/paired/paired-build-v016-success.txt`. The runner has not been executed.

## Next action

When a valid MAIN-1 performance lease is available, qualify the listed shapes independently with their exact Parent binary, then run registered interleaved pairs only for shapes that pass. Exclude FP32 D=16384 without blocking the other shapes. Until timing evidence exists, V016 remains pending and must not receive another performance edit.
