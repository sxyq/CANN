# REDUCE-INVSCALE-X V002 — optional repair-cost recheck, device 4

Policy: Main policy update 2026-09-24T04:03:00Z (optional recheck only).
Candidate source untouched. Frozen SHA re-verified before and after the run:

```text
bef271b62a2c7f2d0b0ef23f5f3610129460f5a431d7d9b3dd7ac3ea9a80ad26
```

(local workspace, local evidence record, and server3 remote all match.)

## Window

```text
device            : ASCEND_DEVICE_ID = 4  (Main-designated PRIMARY)
shape             : FP32 rows=1 D=32768 (6 tiles), identical to the device-6 run
pairing           : v001ref = Direct Parent V001  vs  candidate = V002
pairs             : 4, alternating order, warmups 3, repeats 11 per variant
window opened     : 2026-09-24T04:04:08Z
pairs             : 2026-09-24T04:04:28Z / 04:04:41Z / 04:04:56Z / 04:05:11Z
```

Load on device 4 across the four pairs (from `results_dev4/probe-pair-0*.npu-smi.txt`):

```text
pair 01  AICore 0%   HBM 59259/65536   procs: VLLMEngineCor PID 2999855 (55664 MB),
                                        plus a transient srx_parent_prob (104 MB)
                                        -- another route's probe running on device 4
pair 02  AICore 0%   HBM 59208/65536   VLLMEngineCor PID 2999855
pair 03  AICore 0%   HBM 59209/65536   VLLMEngineCor PID 2999855
pair 04  AICore 0%   HBM 59210/65536   VLLMEngineCor PID 2999855
```

AICore idle throughout, residual VLLM accepted per Main. NPU 0-3 at 32-33%
(VLLMWorker_TP). A co-resident probe process from another route was observed on
device 4 during pair 01.

## Results

| pair | order | V001 µs | V002 µs | Δµs | Δ% | V001 bad | V002 bad |
|---|---|---|---|---|---|---|---|
| 1 | v001ref → candidate | 226.647 | 202.429 | −24.218 | −10.69% | 27133 | 0 |
| 2 | candidate → v001ref | 79.846 | 153.202 | +73.356 | **+91.87%** | 29180 | 0 |
| 3 | v001ref → candidate | 136.914 | 74.737 | −62.177 | **−45.41%** | 28669 | 0 |
| 4 | candidate → v001ref | 98.435 | 146.284 | +47.849 | +48.61% | 28287 | 0 |

- PROBE_DELTAS: −10.69% / +91.87% / −45.41% / +48.61%
- MEDIAN_DELTA: V001 117.675 µs → V002 149.743 µs = **+32.069 µs (+27.25%)**
- WORST_DELTA: +73.356 µs (+91.87%, pair 2); BEST_DELTA: −62.177 µs (−45.41%, pair 3)
- DIRECTIONAL_CONSISTENCY: **inconsistent — 2 faster / 2 slower / 0 tie**
- NOISE_MARGIN: V001 range 146.801 µs (124.8% of its median), V002 range
  127.692 µs (85.3% of its median); |MEDIAN_DELTA| / max range = **0.218 →
  NOT_ABOVE_NOISE**
- CORRECTNESS during probes: candidate `bad=0` in all four pairs;
  Direct Parent V001 `bad≈27-29k` in all four (as expected — V002 repairs it)

## Dominant confound found: position effect inside each pair

In **4 of 4 pairs the second variant measured faster**:

```text
pair 01 second = candidate  -> faster
pair 02 second = v001ref    -> faster
pair 03 second = candidate  -> faster
pair 04 second = v001ref    -> faster
```

The sign of every pairwise delta flips with run order, not with the kernel.
Whatever the real cost of the added `SyncVectorToMte2()` is, it is completely
masked by a first/second-in-pair bias of order 10^2 µs.

## Verdict

**PARK retained.** The device-4 PRIMARY window did not resolve the repair cost:
still below noise, and now additionally confounded by a perfect order effect.
No change to Main's standing decisions:

- V002 source remains frozen at `bef271b6…ad26` (no edit).
- Repair-cost measurement stays PARK.
- R019 normalization performance measurement stays DEFERRED.
- No V003, no CANNJudge.

Evidence: `results_dev4/` (raw probes.tsv, per-pair npu-smi snapshots, per-run
tsv/stdout/output bins). The archived device-6 run in `results/` is untouched.

Suggested next attempt (for Main, not acted on): pair interleave with a
discarding warm-up of both binaries between pairs, or measure each variant in
its own isolated process block, before spending another window on this number.
