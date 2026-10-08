# V047 Result

- Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`).
- Change: in resident multi-row FP16 `ProcessNarrowMidOverlap`, multiply cached gamma into a native-half scratch by `half(invRms)`, then multiply the output by that scratch. The one-row branch, bias add, event schedule, other dtypes, and other paths are unchanged.
- Compile: PASS for `device` and `submission` on `hwnput3`, Ascend910B3 / `dav-2201`, toolkit `8.5.0.alpha002`.
- Correctness: Parent and Candidate PASS on FP16 `[128,2050]`, 40 blocks, 3-4 rows/core. Dispatch selected `ProcessNarrowMidOverlap_FP16`, `resident_params=true`; Candidate source delta executed. Both matched ratio `1.0`; max absolute error `0.00390625`.
- Local: 128 ACL device-event samples per arm, 20 warmups per process, four interleaved Parent/Candidate blocks. Pooled medians: Parent `19.440001 us`, Candidate `17.17 us`; ratio-of-medians score `113.2207396622`, delta `-11.6769592759%` (Candidate faster descriptively).
- Pooled means: Parent `19.0918750937 us`, Candidate `16.5720311484 us`. Pooled CV: Parent `0.3276786689`, Candidate `0.3580004637`.
- Paired median deltas, positive is slower: `-28.8888910837%`, `-20.9640809240%`, `-8.1226465842%`, `-13.9810450237%`. Candidate median was lower in all four pairs.
- Local verdict: `MEASUREMENT_BLOCKED`, not reliable acceptance/rejection or promotion. Both arms show broad, high-variance timings; Parent block medians vary from `16.20` to `21.99 us`. Preserve the descriptive positive score but keep Local Best at V026.
- Load before Local: device 0 healthy, AICore `0%`, HBM `3432/65536 MB` (`62104 MB` free), no process observed on NPU 0. Host load average `30.92, 39.57, 41.11`.
- The initial correctness launch attempt exited 127 because `LD_LIBRARY_PATH` was empty and failed to load `libmsprofiler.so` before program entry. The unchanged binaries passed both Parent and Candidate correctness after setting the proven toolkit/driver library path; initial and retry logs are preserved.
- Device 0 remained exclusively reserved for V047 through numeric result capture. Final release snapshot/receipt follows. Official score absent; Online not run.
- Device 0 was explicitly released after capture at `2026-10-08T02:07:54Z`; final snapshot reports 0% AICore, 3432/65536 MB HBM used, and no process on NPU 0.

Evidence: compile, correctness, Local build logs, initial and retry runner outputs, all eight raw blocks, load snapshots, source hash, declaration, and OFAT diff are retained in this directory.
