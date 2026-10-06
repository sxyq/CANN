# R-W4-4 V001 declaration

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V001
- DIRECT_PARENT: official R31B/V011 saved artifact
- PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
- PARENT_SCORE: 45.16 (Official)
- SINGLE_HYPOTHESIS: Lower only the existing low-precision contiguous-dispatch width cap from 2048 to 1024. For eligible low-precision rows with width in (1024, 2048], dispatch then falls through to the already-existing mid-row path; all kernel implementations and ownership remain unchanged.
- CONTEXT_CLASS: W4_COMPILE_SWEEP
- WHY_NOT_DUPLICATE: Planning explicitly authorized this cutoff campaign. No earlier R-W4-4 evidence is present in the current canonical records; this declaration makes no cross-route novelty claim.
- CHANGE: `kSmallLowPrecisionContiguousMaxWidth`, 2048 -> 1024 (one width cutoff only)
- GATES: compile required; local gate suspended for W4; online forbidden
- NEXT_EDIT_SLA: start the next single-factor edit no later than 180 seconds after Compile PASS
- OWNERSHIP: `/home/data4t2/lelinfeng/cann-r-w4-4` only
