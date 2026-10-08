# V045 Result

- Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`).
- Change: In FP32 `ProcessNarrowMidOverlap`, apply the existing `invRms` multiply to the freshly loaded gamma tile when `localRows==1`; preserve the existing output-value scale for resident multi-row reuse.
- Compile: PASS for `device` and `submission` on `hwnput3`, Ascend910B3 / `dav-2201`, toolkit `8.5.0.alpha002`.
- Correctness: Parent and Candidate PASS on device 0 for FP32 `[40,2050]`, 40 blocks, one row/core. Both matched ratio `1.0`; both max absolute error `1.54972076e-06` under `atol=1.52587891e-05`, `rtol=0.0009765625`, and max-error limit `0.00999999978`. Candidate dispatch audit confirmed the source delta executed in `ProcessNarrowMidOverlap`.
- Local: 96 device-event samples per arm, 20 warmups per process, three interleaved Parent/Candidate blocks. Pooled medians were Parent `16.8100005 us` and Candidate `18.1500005 us`; descriptive score `92.6170800932`, delta `+7.9714453310%` (Candidate slower).
- Paired block deltas (positive is slower): `+28.16457%`, `+17.54177%`, `-12.888667%`. Parent block medians were `15.7999995`, `8.38`, and `19.9399995 us`; Candidate block medians were `20.250001`, `9.8500005`, and `17.3699995 us`.
- Local verdict: `MEASUREMENT_BLOCKED`, not a reliable rejection or promotion. Pooled CV was `0.426175` Parent / `0.448361` Candidate; Parent first-to-last block median drift was `+26.202533%`, with non-monotonic block medians and mixed paired direction. All raw samples are retained; none were excluded. `CURRENT_LOCAL_BEST=V026`.
- Load: device 0 was healthy, AICore `0%`, HBM `3432/65536 MB` (62104 MB free), and no process before or after. Host load averages rose from `34.05,39.52,43.84` to `48.12,45.49,45.38`; other devices had observed activity. No other process was modified.
- Device 0 was reserved exclusively for V045 from correctness through numeric result capture. Reservation/release snapshots and receipts are retained. The initial correctness CMake configure failed before building because `ASC_DIR` was omitted; the failed log is preserved, and the retry using the known-good V042 route-local package path built both runners successfully.
- Official score absent; Online not run.

Evidence: compile, failed and retried correctness build, Parent/Candidate correctness runs, Local build and raw samples, load snapshots, source hash, declaration, and OFAT diff are retained in this directory.
