# LEASE L005 — controlled R006 / R019 orthogonality measurement, device 4

Main lease issued 2026-09-24T04:21:19Z. Serial device-4 lease for
REDUCE-INVSCALE-X only. Candidate source untouched.

## 1. Frozen source (verified before and after the run)

```text
V002 sha256 = bef271b62a2c7f2d0b0ef23f5f3610129460f5a431d7d9b3dd7ac3ea9a80ad26
  local workspace   OK
  local evidence    OK
  server3 remote    OK
V001 (Direct Parent) sha256 = f017935d840bea5a81268d9fe137f1567df0982f4f71718f65f4672ad8da8023  OK
FULL-R006-V001     sha256 = 94ab0ef96a1a907fa187797b6c72361b2bdacfe6faf7a95d6471bd537a913266  OK
git HEAD = ae46d7c, no commits, no add, no push
```

No source edit, no V003, no R019 redesign, no CANNJudge.

## 2. Which arm was used

Main's preferred arm requires **both** variants to pass NPU correctness on the
chosen shape. From the archived 16-shape matrix (`results/correctness-summary.tsv`):

```text
rows=1 D=6144 FP32:  parent bad=0  max_abs 5.44672e-07   PASS
                     candidate bad=0  max_abs 5.44672e-07 PASS
```

Both PASS → **preferred arm used, fallback NOT needed**:

```text
PARENT    = FULL-R006-V001-REDUCTION-ARCH   (R006, Duplicate+Div, no invscale)
CANDIDATE = REDUCE-INVSCALE-X V002          (R006 + R019 invscale + sync fix)
shape     = FP32 rows=1 D=6144              (single tile, both correct)
```

Correctness re-confirmed inside this run: `bad=0` for all 12 probe executions
(6 pairs x 2 variants), `max_abs = 5.44672e-07` on every run.

**Labelling note (per Main's instruction):** V002 differs from FULL-R006-V001
by **two** things — the R019 invscale replacement *and* the V002
`SyncVectorToMte2` repair. This arm therefore measures
"R006 vs R006+invscale+sync", not invscale alone. At D=6144 (single tile) the
sync component is exactly one extra V_MTE2 rendezvous. No R019 gain is claimed
from the fallback arm because the fallback was not run.

## 3. Lease window (preflight / per-pair / postlease)

Full table: `results_L005/L005-LEASE.tsv`.

```text
preflight  2026-09-24T04:23:00Z  DEVICE_ID=4 AICORE=0% HBM=59186/65536 FREE_HBM=6350MB
                                  PROCS=PID 2999855 VLLMEngineCor 55664MB
                                  next6_probe_procs=none          -> no abort
pair 1..6  04:23:04 .. 04:24:16  DEVICE_ID=4 AICORE=0% (all six)
                                  HBM 59186-59188/65536, FREE_HBM 6348-6350MB
                                  PROCS=PID 2999855 VLLMEngineCor 55664MB (all six)
                                  next6_probe_procs=none (all six)
postlease  2026-09-24T04:24:31Z  DEVICE_ID=4 AICORE=0% HBM=59188/65536 FREE_HBM=6348MB
                                  PROCS=PID 2999855 VLLMEngineCor 55664MB
                                  next6_probe_procs=none
```

- Preflight abort guard: **not triggered** (no other next6 probe on any device).
- Residual VLLM on d4: `VLLMEngineCor` PID 2999855, 55664 MB, present throughout
  — accepted per Main's standing note.
- d4 AICore 0% for the whole lease; no cross-route probe observed this time.
- Device 4 released after the run: no REDUCE-INVSCALE-X process remains, no
  next6 probe process on any device (verified 2026-09-24T04:26:22Z).

## 4. Interleaved pairs (6, PC/CP alternating)

Full table: `results_L005/L005-PROBES.tsv`. Shape, device, warmups (3) and
repeats (11) identical for both variants in every pair.

| pair | order | PARENT (FULL-R006) µs | CANDIDATE (V002) µs | Δµs | Δ% | parent bad | cand bad |
|---|---|---|---|---|---|---|---|
| 1 | P→C | 80.405 | 225.343 | +144.938 | +180.26% | 0 | 0 |
| 2 | C→P | 119.726 | 223.333 | +103.607 | +86.54% | 0 | 0 |
| 3 | P→C | 130.827 | 232.822 | +101.995 | +77.96% | 0 | 0 |
| 4 | C→P | 180.909 | 150.218 | −30.691 | −16.96% | 0 | 0 |
| 5 | P→C | 125.566 | 207.800 | +82.234 | +65.49% | 0 | 0 |
| 6 | C→P | 132.306 | 120.496 | −11.810 | −8.93% | 0 | 0 |

## 5. Metrics

```text
PARENT    times : 80.405 119.726 130.827 180.909 125.566 132.306
PARENT    median: 128.197 µs   range: 100.504 µs   jitter: 78.4% of median
CANDIDATE times : 225.343 223.333 232.822 150.218 207.800 120.496
CANDIDATE median: 215.567 µs   range: 112.326 µs   jitter: 52.1% of median

MEDIAN_DELTA : +87.370 µs  (+68.15%)   candidate SLOWER than parent
WORST_DELTA  : +144.938 µs (+180.26%)  pair 1
BEST_DELTA   : −30.691 µs  (−16.96%)   pair 4
DIR          : 4 pairs candidate slower / 2 pairs candidate faster / 0 tie
SAME_DIR     : 4/6 = 0.667
NOISE_RATIO  : |median delta| / max(parent range, candidate range)
             = 87.370 / 112.326 = 0.778
```

**LOAD_QUALITY thresholds used** (stated because Main left them to the route):

```text
CLEAN              : parent jitter <= 10% of parent median
MODERATE           : 10% < parent jitter <= 30%
LOAD_CONTAMINATED  : parent jitter > 30%
```

**LOAD_QUALITY = LOAD_CONTAMINATED** (parent jitter 78.4%).

## 6. Gate evaluation (Main's rule 6)

| gate | required | measured | pass |
|---|---|---|---|
| correctness both PASS | yes | parent 0 bad, candidate 0 bad, 12/12 runs | YES |
| LOAD_QUALITY | CLEAN or MODERATE | **LOAD_CONTAMINATED** (78.4%) | **NO** |
| same direction | >= 3/4 (>=0.75) | 4/6 = 0.667 (first-4 view 3/4) | **NO** |
| gain >> jitter | yes | no gain measured; direction is a slower candidate and noise ratio 0.778 | **NO** |

## 7. Handoff

**NEEDS_ONE_MORE_LOCAL**

- Not `ONLINE_CANDIDATE`: three of the four gates fail (load quality, direction
  fraction, no gain above jitter).
- Not `LOCAL_REJECTED`: the apparent regression is **not stable** — direction is
  4/2 with one pair reversing by −16.96%, LOAD_QUALITY is CONTAMINATED, and
  noise ratio 0.778 means the median delta does not clear the jitter band. A
  stable regression cannot be declared from this window.
- Not a technical failure: no pollution abort, no compile/link problem, no
  correctness failure. Preflight and postlease were both clean of other next6
  probes.

**What this window did establish**

1. Preferred arm is available: FULL-R006-V001 and V002 both pass correctness at
   D=6144, so a genuine R006-vs-R006+invscale comparison is possible without
   falling back.
2. Even on the designated PRIMARY device with AICore at 0% for all six pairs
   and FREE_HBM ~6.3 GB, parent jitter is 78.4% of its own median. The binding
   limit on this machine is no longer AICore occupancy; it is something else
   (HBM-resident VLLMEngineCor at 55.6 GB, host scheduling, or the measurement
   harness itself).
3. Candidate medians (120-233 µs) sit mostly above parent medians (80-181 µs),
   which is directionally consistent with the extra `SyncVectorToMte2` rendezvous
   costing something — but the evidence is below Main's own acceptance bar.

**Suggested next step (not acted on):** before spending another lease, change
the harness rather than the window — discard-warm both binaries between pairs,
or measure each variant in its own isolated block, and/or add a third arm
(V001) so the invscale term and the sync term can be separated. With parent
jitter at 78% on an idle-AICore device, more pairs on the same harness will not
cross the gate.

## 8. Files

```text
phase4/local/REDUCE-INVSCALE-X/V002/support/
  L005-REPORT.md                     this document
  run_probes_L005.sh                 lease harness (copied from workspace)
  results_L005/
    L005-LEASE.tsv                   preflight / per-pair / postlease load
    L005-PROBES.tsv                  6 interleaved pairs
    L005-METRICS.json                machine-readable metrics
    lease.tsv                        raw script lease log
    preflight.npu-smi.txt  postlease.npu-smi.txt
    probe-pair-{1..6}.npu-smi.txt
    probe-pair-{1..6}-{parent,candidate}.tsv / .stdout.txt / -output.bin
```

The earlier device-6 run (`results/`) and the device-4 repair-cost recheck
(`results_dev4/`) are untouched.
