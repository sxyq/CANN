# STORE-EPILOGUE-W2-X V001 Correctness Diagnostic Run 004

DIAGNOSTIC_STATUS=UNRESOLVED_BOTH_VARIANTS_NONDETERMINISTIC
CORRECTNESS_STATUS=INCOMPLETE
CORRECTNESS_CONCLUSION=UNRESOLVED
TIMING_ELIGIBILITY=INELIGIBLE_CORRECTNESS_NOT_PASSED
RUN_END=2026-10-02T16:47:13+00:00

## Identity and Scope

- Route branch: `w2/m1/store-epilogue`; Candidate source commit: `a1ebfc125ce215fc81ffbe04f582d5f8682e3294`.
- Candidate source SHA: `48b9428dc2fc97c7c9d95f03ad8cec8e758c88e1197aa2328b1b1edebc018f88`.
- Direct Parent: `STORE-EPILOGUE-X V002`.
- Run-004 covered device 5, `rows=1`, `width=32768`, `dtype=0` (FP32). It made two calls per variant in order Parent 1, Candidate 1, Parent 2, Candidate 2.
- The driver ended at `2026-10-02T16:47:13+00:00` after recording all four results, output comparisons, and the post-run device snapshot.

## Inputs

- `correctness_main.inc` generates inputs with fixed arithmetic formulas for X, residual, gamma, and beta; it uses no random or time-based seed. For the same shape and dtype, those formulas produce the same input values for each call.
- All four run-004 TSVs record device 5, `rows=1`, `width=32768`, and `dtype=0`. The run-004 driver records one set of source-equivalent H2D buffers shared by the calls.
- The run-003 diagnostic identifies the same generator source. Run-002 and run-003 driver logs do not contain equivalent per-run input-generation records, so exact payload identity across those run directories cannot be independently attested from their logs. No input SHA was added here.

## Run-004 Results

Each output was 131072 bytes. All four calls returned `3`, the runner code for a failed host expected-value comparison.

| Call | RC | max_abs | bad | gross_bad | elements |
|---|---:|---:|---:|---:|---:|
| Parent rep1 | 3 | 1.19637022 | 21098 | 18017 | 32768 |
| Candidate rep1 | 3 | 0.807902592 | 23185 | 22265 | 32768 |
| Parent rep2 | 3 | 1.11338508 | 29274 | 22269 | 32768 |
| Candidate rep2 | 3 | 1.27309979 | 27118 | 24265 | 32768 |

The four retained TSVs are:

- `parent_r1_d32768_t0_rep1.tsv`
- `parent_r1_d32768_t0_rep2.tsv`
- `candidate_r1_d32768_t0_rep1.tsv`
- `candidate_r1_d32768_t0_rep2.tsv`

Parent rep1 and rep2 are not byte-identical; 94690 bytes differ. Candidate rep1 and rep2 are not byte-identical; 79925 bytes differ. None of the four Parent/Candidate pairings is byte-identical:

| Pair | Differing bytes |
|---|---:|
| Parent rep1 vs Candidate rep1 | 80505 |
| Parent rep1 vs Candidate rep2 | 70295 |
| Parent rep2 vs Candidate rep1 | 96894 |
| Parent rep2 vs Candidate rep2 | 100375 |

The retained-binary comparison also found no run-004 output byte-identical to the 1x32768 outputs retained from run-002 or run-003.

## Comparison with Earlier Runs

- Run-002 at `1x32768 FP32`: Parent and Candidate both returned `3`; their outputs differed. Parent recorded `bad=24441`, Candidate `bad=28017`.
- Run-003 at `1x32768 FP32`: both variants returned `3` on both repetitions. Parent repetitions differed in 28928 elements; Candidate repetitions differed in 29312 elements. The run-003 rep1 Parent/Candidate outputs differed in 28064 elements.
- Run-004 repeats the shared Parent/Candidate instability seen in run-002 and run-003. The varying mismatch counts and outputs do not identify which variant, if either, agrees with an independent reference.

## Conclusion

Correctness is unresolved. Parent and Candidate each vary across repeated calls, and all four run-004 calls fail the host expected-value comparison. This does not establish a Candidate-only defect, and it does not establish a correctness pass for either variant. Neither variant has timing eligibility; do not run timing on this evidence.

No timing, test rerun, Candidate edit, or Revision edit was performed for this record. Earlier run-001, run-002, and run-003 records remain unchanged.

## Evidence

- Driver: `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-004.log`
- Run-004 outputs, per-call logs, and TSVs: `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-004/`
- Earlier run-002 record: `本地实验/STORE-EPILOGUE-W2-X/V001/CORRECTNESS-RUN-002-RESULT.md`
- Earlier run-003 diagnostic: `本地实验/STORE-EPILOGUE-W2-X/V001/CORRECTNESS-DIAGNOSTIC-RUN-003.md`
