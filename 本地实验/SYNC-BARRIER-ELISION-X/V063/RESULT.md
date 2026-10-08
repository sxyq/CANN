# SYNC-BARRIER-ELISION-X V063 Result

## Revision and qualification

- Route/revision: `SYNC-BARRIER-ELISION-X` / `V063`.
- Direct Parent and Local Best: exact `R31B-V011`.
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate source SHA256: `57f9b176c1c444ada9f58ffd8741ca3655ac47cc9c25518b921115908265c013`.
- Single change: delete the `SyncVToS()` immediately after `SyncVToMTE2()` before the `meanSquares` scalar extraction loop in `ProcessSmallLowPrecisionContiguousBatched`; no other kernel operation changed.
- Compile: `PASS` for `sync_barrier_elision_v063` and `sync_barrier_elision_correctness`. Earlier failed build attempts are preserved; the successful host-include fix is in `logs/compile-v063-hostinclude-fix.log`.
- Correctness: `PASS`, 7/7 FP16 cases bitwise equal; see `logs/correctness-v063.log`.
- Same-binary qualification: Parent and Candidate each completed 31 samples. Parent first/second medians were 18.54/18.00 us, with CV 0.39640/0.33810 and median drift 0.029557. Candidate medians were 7.00/7.42 us, with CV 1.03079/0.74453 and median drift 0.058252. Raw logs: `logs/qualification-v063-parent.log` and `logs/qualification-v063-candidate.log`.

## Local capture

- Device: NPU 3, Ascend910B3; shape `128x128`, FP16; device-event latency is the scored metric and wall latency is diagnostic.
- Runner: `/tmp/sync-v063-build/sync_barrier_elision_correctness`; source and executable identities are recorded in the pre-capture snapshots. Runner source labels are `PARENT_R31B_V011` and `CANDIDATE_V063`.
- Method: 60 warmups per invocation; two interleaved blocks of 31 P/C pairs each; alternating order; all 62 pairs retained. No sample was dropped.
- Raw samples: `logs/local-v063-block1.log` and `logs/local-v063-block2.log`. Aggregates below were independently recomputed from the 4-decimal raw event values in those files.
- Block 1: Parent median `14.90 us`, Candidate median `16.80 us`, paired-delta median `+0.46 us`; paired-median score `-3.087248%`, median-latency speedup `-11.309524%`.
- Block 2: Parent median `17.06 us`, Candidate median `15.86 us`, paired-delta median `-0.30 us`; paired-median score `+1.758499%`, median-latency speedup `+7.566204%`.

## Pooled numeric result

- Pooled Parent device latency: median `16.33 us`, mean `14.189355 us`, stdev `6.977453 us`, CV `0.491739`, MAD `3.04 us`, range `5.94-43.54 us`.
- Pooled Candidate device latency: median `16.14 us`, mean `13.722903 us`, stdev `6.617618 us`, CV `0.482232`, MAD `3.92 us`, range `5.82-40.00 us`.
- Paired Candidate-minus-Parent delta: median `-0.12 us`, mean `-0.466452 us`, stdev `10.615622 us`, MAD `3.91 us`, range `-37.10 to +33.66 us`.
- Primary paired-median Local score: `+0.734844%`, computed as `-100 * median(paired Candidate-minus-Parent delta) / median(Parent latency)`.
- Separate pooled median-latency speedup: `+1.177200%`, computed as `100 * (Parent median / Candidate median - 1)`. Candidate-minus-Parent pooled median latency change is `-1.163503%` of Parent median. Mean-latency speedup is `+3.287335%` and is reported only as an outlier-sensitive diagnostic.
- Pooled wall-time medians: Parent `70.216 us`, Candidate `69.5415 us`; means `75.067016/72.399274 us` respectively. Pooled per-sample throughput medians: Parent `1.003337 Gelem/s`, Candidate `1.015424 Gelem/s`.

## Noise, load, and verdict

- The two blocks disagree in direction: block 1 regressed while block 2 improved. Pooled paired-delta stdev is `10.615622 us`, versus a pooled median paired delta of only `-0.12 us`; block 1 alone had paired-delta stdev `13.183442 us` and max absolute latency outliers above `40 us`.
- Device snapshots show NPU 3 at 5% HBM usage, 0% AICore, and no device process before the Candidate qualification and before both Local blocks; the post-capture snapshot also shows no device process. Host load was high and changed during capture: the last pre-block-1 snapshot at `13:19:18Z` reported load averages `39.37/51.46/54.22`; the pre-block-2 snapshot at `14:11:36Z` reported `62.40/56.93/57.91`; post-capture at `14:14:33Z` reported `67.64/59.48/58.57`. The pre-block-1 snapshot preceded that block by about 51 minutes, so immediate block-1 host load is unknown. Snapshot logs are preserved under `logs/device3-v063-*.log`.
- Device 3 assignment was explicitly released at `2026-10-08T14:14:35.092558893Z` after raw capture; see `logs/device3-v063-post-local-release-snapshot.log`.
- Verdict: `LOCAL_REJECTED_NOISY`. The small pooled positive score is not repeatable across blocks and does not justify promotion. `CURRENT_LOCAL_BEST` remains exact `R31B-V011`.
- This single-shape Local measurement is not comparable to Official Score `45.16`; no Official score was measured and no Online action was taken.

## Evidence identities

- Kernel executable SHA256: `93b3ad342061c645c10db7f60ca42a5765fcb522fd4a17d6fc2b8765b4e257b0`.
- Correctness/Local runner executable SHA256: `b84dbd8bd3f382ddfd499fb23b50aad7b782fd97000dcc3cd2d42fb0fd85fa8f`.
- All raw qualification/Local outputs, resource snapshots, compile attempts, and correctness output are retained in `logs/`.
