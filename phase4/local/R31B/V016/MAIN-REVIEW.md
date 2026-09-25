# R31B V016 Main Review

Date: 2026-09-25

## Decision

`NEEDS_ONE_MORE_LOCAL`. Keep V016 unchanged. Do not create V017, promote, reject, or submit online from this evidence.

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
- The FP32 D=16384 control failed for both V011 and V016 under the same harness. This is outside the changed dtype path and is recorded as inherited or harness-related; it is not evidence to accept or reject V016.
- Four earlier paired observations are `LOAD_CONTAMINATED`; no performance conclusion follows from them.
- The unified paired runner now compiles and links. Parent, Candidate, wrapper, runner sources, CMake input, and runner executable identities are recorded in `route-handoff.md` and `support/paired/paired-build-v016-success.txt`. The runner has not been executed.

## Next action

Wait for a current MAIN-1 exclusive device lease and a comparable load window. Qualify each exact shape using the same Parent binary, then run the registered interleaved pairs only for shapes that pass. Until then V016 remains pending and must not receive another performance edit.
