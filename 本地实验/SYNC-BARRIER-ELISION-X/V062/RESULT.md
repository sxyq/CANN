# SYNC-BARRIER-ELISION-X V062 Result

- Direct Parent: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`. V061 was not inherited.
- Single Candidate change: delete only `WaitFlag<V_MTE3>(outputReady)` immediately before the batched output `Store` in `ProcessSmallLowPrecisionContiguousBatched`. Parent/Candidate diff is exactly this one-line deletion. Candidate source SHA256: `25addc5b93d805ba0c4c1e0b531ddd2d81b89e20d9f85981664773e3a9568582`.
- Compile: PASS for `sync_barrier_elision_v062` and `sync_barrier_elision_correctness`. Build directory was `/tmp/sync-v062-build.7ZVF6R`; target mtimes were 12:27:37Z and 12:28:02Z. The adapter target SHA256 is `db79236f011bfcff8786d48503a6bab38b9acb2c1d615793b94e6efc64b752ec`; it matches V061 because this support adapter has a no-op `main` and is not evidence of Candidate-specific device code. The correctness runner SHA256 is `66a9fe24ca3f7ab1dfd1b45c70ee951ca5e1f86dde17b207226975a14e00b69e`, with route labels `CANDIDATE_V062` and `PARENT_R31B_V011`. The compile transcript itself was not retained in this revision's `logs/`; the successful target artifacts and their identities are recorded here.
- Correctness: PASS, all 7 FP16 cases bitwise equal; raw output is `logs/correctness-v062.log`.
- Same-binary qualification: 31 samples for each binary. Parent device-time CVs were 0.36593/0.34902 with median drift fraction 0.014218. Candidate CVs were 0.65598/0.62500 with median drift fraction 0.032305. Raw qualification records are retained in `logs/qualification-v062-parent.log` and `logs/qualification-v062-candidate.log`.

## Local

FP16 128x128, device 3, 62 interleaved Parent/Candidate pairs in two retained 31-pair blocks. Device-event latency and throughput samples, wall time, pair order, and per-block statistics are preserved in `logs/local-v062-raw.log` and `logs/local-v062-block2-raw.log`.

The paired-median score is `-median(C-P) / median(P) * 100`; the separate median-latency ratio is `(median(P)-median(C)) / median(P) * 100`. Latencies and paired deltas are in microseconds; throughput is GElem/s.

| Set | P median us | C median us | Median C-P us | Paired-median score | Median-latency ratio | P/C mean us |
|---|---:|---:|---:|---:|---:|---:|
| Block 1, 31 pairs | 15.6400 | 15.1800 | +0.1000 | -0.639386% | +2.941176% | 13.1265 / 12.8065 |
| Block 2, 31 pairs | 17.1000 | 16.3200 | -0.0800 | +0.467836% | +4.561404% | 15.3723 / 15.7148 |
| Pooled, 62 pairs | 16.3100 | 15.7800 | +0.0700 | -0.429185% | +3.249540% | 14.2494 / 14.2606 |

Pooled device-time sample standard deviations were 5.9102 us (Parent) and 6.3467 us (Candidate), with CVs 0.41477 and 0.44505. Pooled wall-time medians were 62.8055/63.1760 us; pooled median throughput was 1.0045/1.0383 GElem/s. The block paired-median scores disagree in sign. Although the separate pooled median-latency ratio is positive, paired median delta is +0.0700 us and Candidate pooled mean latency is 0.0112 us higher. Qualification jitter, high Local CV, and block disagreement make the result non-repeatable evidence. Verdict: `LOCAL_REJECTED_NOISY`; no promotion. Local Best remains exact `R31B-V011`.

Device 3 snapshots before qualification and both Local blocks showed 3,428/65,536 MB HBM used (62,108 MB free), 0% AICore, and no NPU 3 process. The post-capture snapshot is `logs/device3-v062-postcapture-snapshot.log` (captured 2026-10-08 12:38:09.063625245Z); it again shows 0% AICore, the same HBM usage, and no NPU 3 process. The V062 device-3 assignment is explicitly released as of `2026-10-08T12:44:15.998731314Z`, after raw capture and result calculation. No shared lease TSV was modified.

SLA record: the V061 result-to-edit interval cannot be stated exactly because the numeric-result timestamp was not separately persisted. From the final V061 raw-block completion marker at `2026-10-08T12:09:53.904472160Z` to the V062 Candidate edit mtime `2026-10-08T12:26:56.727677881Z` is 17m02.823s; this exceeds the 180-second target. This is the marker-to-edit interval, not a claim about the missing exact result timestamp.

This single-shape route-local measurement is not comparable to Official Score 45.16. No Online, shared-record write, or push was performed.
