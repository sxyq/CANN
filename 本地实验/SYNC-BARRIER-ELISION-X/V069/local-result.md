# V069 Local Result

## Identity and gates

- Route: `SYNC-BARRIER-ELISION-X`
- Revision: `V069`
- Parent and Local Best: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate SHA256: `ab23dd2b5b36289dd522d550abc768a6371ed74d3033fc8740067c4a26f365b9`
- Correctness runner SHA256: `700d8830fa2af0abd4ff1efa50b515a8cb98fc159729f7602ec275d461f66e0c`; embedded labels verified as `CANDIDATE_V069` and `PARENT_R31B_V011`.
- Compile: PASS for the V069 kernel target and correctness runner. Initial attempts and their unmodified logs are retained: first lacked `ASCEND_HOME_PATH`; the next exposed `<vector>` missing in Bisheng's generated host plugin. Final build passed with `ASCEND_HOME_PATH` plus `CPLUS_INCLUDE_PATH` set to the existing toolkit/GCC C++ headers. No Kernel hypothesis change was made for these support fixes.
- Correctness: PASS, 9/9 cases on device 3; all 8 FP16 cases and the BF16 case reported zero parent/candidate bit mismatches.

## Measurement

- Shape/dtype: `128x256`, BF16; this single-shape Local is not comparable to Official 45.16.
- Device: 3. Parent and Candidate same-binary qualifications each retained 31 pairs, followed by two interleaved Parent/Candidate blocks of 31 pairs each (62 raw pairs total). All samples, including outliers, are retained in `logs/qualification-parent.log`, `logs/qualification-candidate.log`, `logs/local-block1.log`, and `logs/local-block2.log`; `logs/derived-pairs.tsv` contains all 62 raw-derived rows.
- Raw device-event latency medians across all pairs: Parent `16.7400 us`, Candidate `10.8500 us`; means `14.330323 us` and `14.767097 us`; standard deviations `5.875985 us` and `9.037477 us`.
- Raw device-event throughput medians: Parent `1.958585 Gelem/s`, Candidate `3.060716 Gelem/s`; per-sample throughput values are retained in the block logs and derived table.
- Paired latency delta is `Candidate - Parent`. Across all 62 pairs its median is `+0.4900 us`, mean `+0.436774 us`, standard deviation `10.050308 us`, range `-17.3400` to `+48.5000 us`; Candidate was faster in 27 pairs and slower in 35.
- Runner paired score formula: `-100 * median(Candidate - Parent) / median(Parent)`. Result: `-2.927121%` (negative means slower by the paired-median measure).
- Separate pooled-median latency ratio: `100 * (median(Parent) / median(Candidate) - 1) = +54.285714%`. This conflicts with the paired score and with the mean-latency change (`+3.047901%` Candidate latency), so it is not treated as a reliable gain.
- Wall-time medians: Parent `73.1510 us`, Candidate `75.6955 us`; means `78.496629 us` and `76.001694 us`. Wall-time and device-event summaries also disagree in direction.

## Block and Noise Summary

| Block | P median (us) | C median (us) | Median paired delta (us) | Pooled-median latency ratio | Runner paired score |
|---|---:|---:|---:|---:|---:|
| 1 (31 pairs) | 16.3400 | 15.1600 | +0.3000 | +7.783641% | -1.835985% |
| 2 (31 pairs) | 17.1400 | 9.2000 | +0.6400 | +86.304348% | -3.733956% |

Parent qualification had 25.6% median drift between its two same-binary runs, with device-event CVs `0.53/0.42`. Candidate qualification had device-event CVs `0.87/0.43` and a wall-time outlier of `7224.018 us`. During Local, pooled device-event CVs were `0.410/0.612`; paired-delta spread was large relative to its median, the blocks differed substantially, and pooled median-ratio and paired/mean measures disagree. Load snapshots show device 3 at `9131/65536 MB` HBM used (about `56405 MB` free); an existing Python process (PID `2055830`, `5756 MB`) remained present and was not touched. The two Local pre-block snapshots recorded AICore `0%` and `10%`; the post-capture snapshot recorded `7%`. Full snapshots are retained under `logs/`.

## Verdict and Release

- Verdict: `LOCAL_REJECTED_NOISY`; numeric values are retained, but V069 is not promoted and Local Best remains exact `R31B-V011`.
- Device 3 use ended after the second block; post-capture snapshot and release timestamp `2026-10-08T17:46:56Z` are in `logs/post-capture-release-snapshot.log`. No existing process was disturbed.
- No shared records, Online submission, or push were performed.
