# V046 Result

- Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`).
- Change: In FP32 resident multi-row `ProcessNarrowMidOverlap`, apply the existing `invRms` multiply to `gammaLocal` through the existing `xFp32` scratch, then multiply the output value by that scratch. The cached gamma remains unchanged for later rows; the one-row FP32 and all other paths are unchanged.
- Compile: PASS for `device` and `submission` on `hwnput3`, Ascend910B3 / `dav-2201`, toolkit `8.5.0.alpha002`.
- Correctness: Parent and Candidate PASS on device 0 for FP32 `[128,2050]`, 40 blocks and 3-4 rows/core. Dispatch audit selected `ProcessNarrowMidOverlap`; Candidate source delta executed. Both matched ratio `1.0`; max absolute error `3.09944153e-06`.
- Local: 96 device-event samples per arm, 20 warmups per process, three interleaved Parent/Candidate blocks. Pooled medians: Parent `16.43 us`, Candidate `17.29 us`; descriptive score `95.0260266050`, delta `+5.2343274498%` (Candidate slower).
- Paired block deltas (positive is slower): `-6.305765%`, `+6.662222%`, `+56.741573%`. Parent block medians were `16.810000`, `15.010000`, and `12.460000 us`; Candidate block medians were `15.750001`, `16.010000`, and `19.530000 us`.
- Local verdict: `MEASUREMENT_BLOCKED`, not a reliable rejection or promotion. Pooled CV was `0.338359` Parent / `0.289199` Candidate; Parent block medians drifted down while Candidate medians rose in the final block, with mixed paired direction and a large final-pair gap. All 96 raw samples per arm are retained and included.
- Load: before Local, device 0 was healthy, AICore `0%`, HBM `3432/65536 MB` (`62104 MB` free), with no process on NPU 0. Host load average was `48.45, 43.91, 44.62`.
- Candidate block 1's runner returned 0 and printed correctness plus raw samples, while the capture wrapper returned 1 because of a destination-path typo. The exact stdout is retained at its canonical path with the capture note; no samples were discarded.
- Device 0 remained reserved exclusively for V046 through numeric result capture. Release snapshot/receipt follows. Official score absent; Online not run.

Evidence: compile, failed and retried correctness-runner builds, Parent/Candidate correctness runs, Local build and six raw blocks, load snapshots, source hash, declaration, and OFAT diff are retained in this directory.
