# R-W4-4 V036 revision declaration

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`
- REVISION: `V036`
- DIRECT_PARENT: exact sibling `R31B-V011`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- CANDIDATE_SOURCE_SHA: `f4b3623b4a8cb86a43328f5dbc15fe6826ab8f13cbcc88745d0e23aa9da2b49f`
- PARENT_SCORE: `45.16` (historical Official metadata only; no Official comparison in this experiment)
- SINGLE_HYPOTHESIS: Change only `kSmallFp32BatchMaxWidth` from `4096` to `1024` to test the next power-of-two width cutoff after the retained 64/128/192/256/512 sweep.
- CONTEXT_CLASS: FP32, 128 rows, widths immediately below/at/above 1024.
- WHY_NOT_DUPLICATE: Existing cutoff experiments do not test 1024; V036 samples the boundary with 1016/1024/1032 using the exact frozen sibling baseline, not a noisy Route Candidate.
- EXPECTED_SHAPES: `128x1016`, `128x1024`, `128x1032` FP32.
- MINIMAL_VALIDATION: Exact-source build/link; Parent/Candidate correctness on those three widths; then Parent same-binary and shape/window qualification before any interleaved Local pairs.
- PARENT_KNOWN_CORRECTNESS_FAILURE: Exact route Parent C15 FP32 `1x32768` remains known to fail; do not rerun or attribute it to V036.
- PARTIAL_CORRECTNESS: `YES` until a separately validated correctness baseline covers the known C15 case; C15 is excluded from this revision.
