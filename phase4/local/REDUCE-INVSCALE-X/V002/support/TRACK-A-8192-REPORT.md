# REDUCE-INVSCALE-X — TRACK-A 1x8192 FP32 same-binary (unified device-event protocol)

Cycle: long-horizon 2026-09-25. Route agent REDUCE-INVSCALE-X (MAIN-2).
One clean attempt, as instructed. No kernel edits, no V003, no CANNJudge.

## SHA verification (before and after the run)

```text
candidate  submission.asc               bef271b62a2c7f2d0b0ef23f5f3610129460f5a431d7d9b3dd7ac3ea9a80ad26  local MATCH / remote MATCH (unchanged)
direct P   submission_v001_reference.asc f017935d840bea5a81268d9fe137f1567df0982f4f71718f65f4672ad8da8023  remote MATCH (= V001)
grand-P    parent.asc                   94ab0ef96a1a907fa187797b6c72361b2bdacfe6faf7a95d6471bd537a913266  remote MATCH (not used this run)
```

## Lease / device

```text
lease   LH-REDUCE-8192  device 6 (d4 taken by ALIGN-TAIL-X, d5 by BATCH-RESIDENT-X -> secondary d6; d7 forbidden)
        LEASED 2026-09-25T14:26:00Z -> RELEASED 2026-09-25T14:37:00Z
window  2026-09-25T14:31:18Z .. 14:36:08Z
load    NPU6 AICore 0% throughout; HBM 59875-59877/65536; resident VLLMWorker_TP PID 91228 (56355 MB) documented (accepted per standing precedent)
```

## Harness

Unified reference harness, verbatim from MAIN-2 (no route-specific method):

- `runner_ref.inc` sha256 `89f8380a5ce6d6538374da67bd8151bf1b529131796a0d747d865a41b6f56fa9`
  (identical to `SCHED-ROWGROUP-X/.../support/runner_ref.inc`), deployed to the route's
  remote workspace; measurement layer only — it `#include`s the unchanged submissions
  unchanged.
- Two probes built from it (build files only; sources untouched):
  - `reduce_invscale_refdirect_probe` → `#define SRX_SUBMISSION "submission_v001_reference.asc"` (Direct Parent V001)
  - `reduce_invscale_refcand_probe`   → `#define SRX_SUBMISSION "submission.asc"` (V002)
- Method: DEVICE_EVENT_PRIMARY + HOST_WALL_SECONDARY, warmup in-process once,
  blocks in one process, `batch_n=1`, no D2H inside the timed loop.
- Build note: two probes must be linked serially (`-j1`) and with
  `CPLUS_INCLUDE_PATH` set to the hcc 7.3.0 C++ headers, otherwise the
  asc plugin registration step fails (`'vector' file not found`) — same CMake
  flags as the existing probe targets otherwise.

## 1) Multi-tile reduction path — runtime confirmation, both binaries

Shape FP32 rows=1 D=8192 → `ceil(8192/6144) = 2` reduction tiles on both binaries
(source trace already in `next-hypotheses.md` PROBE-SHAPE DESIGN; this run adds
runtime confirmation):

| binary | run | result | reading |
|---|---|---|---|
| Direct Parent V001 (refdirect) | same-binary timing run (also does one D2H golden check) | `bad=2048`, `max_abs=5.4174` | **exactly the known V001 multi-tile defect** (bad = D−6144 = 2048, max_abs 5.4174 matches source-meta) → the 2-tile collector/collapse path executes |
| Candidate V002 (refcand) | correctness-confirmation run (warmup 3, 5 samples, 1 block — timing numbers NOT used) | `bad=0`, `max_abs=1.66893e-06` | multi-tile path executes and is correct |

## 2) Same-binary noise floor — Direct Parent V001, FP32 1x8192, d6

Protocol: warmup=10, 2 blocks × 31 samples, gap 2 s, one process, device events.
Raw: `results-ref-8192/samebin-w10-s31-raw.tsv` (62 rows, permanent).

| block | n | median µs | MAD µs | MAD/med | p10 | p90 | min | max |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| B1 | 31 | 14.220 | 7.520 | **0.5288** | 7.22 | 92.58 | 6.70 | 124.70 |
| B2 | 31 | 7.000 | 0.400 | 0.0571 | 6.60 | 37.48 | 6.34 | 537.58 |
| ALL | 62 | 8.330 | 1.790 | **0.2149** | 6.70 | 89.49 | 6.34 | 537.58 |

- **MAD/median (ALL) = 0.2149 > 0.10 → threshold FAIL**
- **block drift = |B2_med − B1_med| / ALL_med = 7.220/8.330 = 0.8667 > 0.10 → threshold FAIL**
- Block drift 0.8667 > 0.25 → per local-timing-protocol this is
  **`MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`** (not a Route failure, not a Candidate
  signal; the harness/host, never the kernel, is what would need to change).
- WALL (diagnostic only): ALL median 141.2 µs, CV 0.588 — judged on device events, not wall.

## 3) Decision

```text
CRITERION: MAD/median <= 0.10 AND block drift <= 0.10   -> UNQUALIFIED (both conditions failed)
CLEAN ATTEMPTS USED: 1 / 1  ->  Track-A stops per brief ("one clean attempt then Track-B")
INTERLEAVED P/C: NOT RUN (0 candidate timing samples this cycle; threshold never met)
VERDICT: NEEDS_ONE_MORE_LOCAL + MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE at FP32 1x8192 d6
V002 SHA: unchanged (bef271b6...ad26)   V001 SHA: unchanged (f017935d...8023)
No kernel edits, no V003, no CANNJudge, no source read/written by the harness beyond #include.
```

Notes:
- The two blocks disagree structurally (B1 bimodal ~7/90 µs, B2 mostly ~7 µs with a
  537 µs tail event) — minute-scale drift under the resident VLLMWorker, consistent
  with the historical d6/d4 pattern; the shape never had a qualified window under
  any method (legacy CV criterion also UNQUALIFIED 2/2).
- B2 alone would pass (MAD/med 0.0571); the pre-registered rule judges the full
  sample set and both blocks — no post-hoc selection of B2 is allowed.

## 4) Files

```text
phase4/local/REDUCE-INVSCALE-X/V002/support/
  TRACK-A-8192-REPORT.md              this file
  results-ref-8192/
    samebin-w10-s31-raw.tsv           62 permanent raw device/wall samples
    samebin-w10-s31-stats.txt         per-block + ALL stats (harness output)
    samebin.stdout.txt / samebin.stderr.txt
    cand-correctness-w3-s5-raw.tsv / -stats.txt / cand.stdout.txt (multi-tile bad=0)
    npu-smi-start.txt / npu-smi-end.txt / start.timestamp.txt / end.timestamp.txt
server3 (remote, same content):
  /home/data4t2/lelinfeng/phase4-workspaces/REDUCE-INVSCALE-X/
    runner_ref.inc runner_refdirect.asc runner_refcand.asc     (measurement layer)
    CMakeLists.txt (+ ref probe targets; CMakeLists.txt.bak-ref = pre-append copy)
    build/reduce_invscale_refdirect_probe, build/reduce_invscale_refcand_probe
    results-ref-8192/*
control: phase4/control/server3-device-leases.tsv  LH-REDUCE-8192 LEASED->RELEASED (d6)
```
