# V048 Result

- Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`).
- Change: in FP16 one-row `ProcessNarrowMidOverlap`, move the native-half row-scale Muls on the output ahead of the parameter-ready wait; gamma multiplication remains after the wait and bias placement is unchanged.
- Compile: PASS for `device` and `submission` on `hwnput3`, Ascend910B3 / `dav-2201`, toolkit `8.5.0.alpha002`.
- Correctness: Parent and Candidate PASS on FP16 `[40,3072]`, 40 blocks, one row/core, `resident_params=false`. Both matched ratio `1.0`, max absolute error `0.00390625`; Candidate source delta executed.
- Local: 128 ACL device-event samples per arm, 20 warmups per process, four interleaved Parent/Candidate blocks. Pooled medians: Parent `19.75 us`, Candidate `16.5200005 us`; ratio-of-medians score `119.552054493`, delta `-16.3544278481%` (Candidate faster descriptively).
- Pooled means: Parent `19.5807813125 us`, Candidate `20.7096875938 us`. Pooled CV: Parent `0.3452198357`, Candidate `1.3817257014`.
- Paired median deltas, positive is slower: `-1.66964515%`, `-16.3876272761%`, `-18.9585444631%`, `-13.4281941561%`. Candidate median was lower in all four blocks.
- Local verdict: `MEASUREMENT_BLOCKED`, not reliable acceptance/rejection or promotion. Candidate timing variance is very high, its pooled mean is slower, and a retained sample spikes to `296.380005 us`. No samples were excluded. Local Best remains V026.
- Load before Local: device 0 healthy, AICore `0%`, HBM `3432/65536 MB` (`62104 MB` free), no process observed on NPU 0. Host load average `24.40, 38.06, 43.41`.
- Device 0 remained exclusively assigned to V048 through numeric result capture, then was explicitly released at `2026-10-08T02:46:17Z`. The post-capture snapshot reports device 0 healthy, AICore `0%`, HBM `3432/65536 MB`, and no process on NPU 0. Online was not run.

Evidence: compile/correctness/local runner builds, three pre-stage device snapshots, parent/candidate correctness outputs, all eight raw Local blocks, numeric aggregate, and failed harness configuration attempts are retained in this directory.
