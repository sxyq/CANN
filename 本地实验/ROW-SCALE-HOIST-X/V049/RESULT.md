# V049 Result

- Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`).
- Change: in BF16 one-row `ProcessNarrowMidOverlap`, move the existing FP32 `valueLocal` row-scale `Muls` ahead of the parameter-ready wait when `residentParams == false`; resident multi-row placement remains unchanged.
- Compile: PASS for `device` and `submission` on `hwnput3`, Ascend910B3 / `dav-2201`, toolkit `8.5.0.alpha002`.
- Correctness: Parent and Candidate PASS on BF16 `[40,3072]`, 40 blocks, one row/core, `resident_params=false`. Both matched ratio `1.0`; max absolute error `0.00781273842` under `atol=rtol=1/64`, max-error limit `1.0`. Candidate dispatch audit confirms the source delta executed in `ProcessNarrowMidOverlap`.
- Local: 128 ACL device-event samples per arm, 20 warmups per process, four interleaved Parent/Candidate blocks. Pooled medians: Parent `17.5700005 us`, Candidate `18.83 us`; ratio-of-medians score `93.3085528412`, delta `+7.1713116912%` (Candidate slower).
- Pooled means: Parent `16.5929687891 us`, Candidate `17.6092188125 us`. Population CV: Parent `0.4656422254`, Candidate `0.3973337599`; retained extrema were Parent `6.54–54.140003 us`, Candidate `6.84–35.16 us`.
- Parent/Candidate block medians were `18.6599995/19.5300005`, `15.76/20.290001`, `10.1899995/9.75`, and `18.26/18.36 us`. Paired median deltas, positive is slower: `+4.6623849052%`, `+28.7436611675%`, `-4.3179540882%`, `+0.5476451260%`.
- Local verdict: `MEASUREMENT_BLOCKED`, not a reliable rejection or promotion. Parent block medians drift substantially and non-monotonically; paired direction is mixed. Pooled median and mean both indicate the Candidate is slower, but the measurement is not stable enough for a reliable decision. All samples are retained with none excluded. Local Best remains V026.
- Pre-Local snapshot at `2026-10-08T03:42:08Z`: device 0 healthy with HBM `4814/65536 MB` (`60722 MB` free), AICore `15%`, AIVector `14%`, and an unrelated Python process using 1434 MB. Per assignment, work proceeded because free HBM exceeded 100 MB; unrelated processes were not changed. Host load average was `65.94, 58.30, 52.20`.
- The first correctness launch attempt exited 127 before program entry because the dynamic loader could not find `libmsprofiler.so`; both unchanged binaries passed after setting the toolkit/driver `LD_LIBRARY_PATH`. Initial outputs are preserved.
- Device 0 was explicitly released after numeric capture at `2026-10-08T03:49:06Z`; the final snapshot shows no V049 runner process. Online was not run.

Evidence: V049 compile, runner build and initial failed build, correctness initial/retry outputs, fresh pre-correctness and pre-Local snapshots, all eight Local raw logs, and the numeric aggregate are retained in this directory.
