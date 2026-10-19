# REDUCE-INVSCALE-X 1x8192 FP32 — warmup=45 re-validation (Track-A, lease W45-REDUCE-8192)

**Decision: MEASUREMENT_BLOCKED** (`MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` — no P/C run)

- Lease `W45-REDUCE-8192` (d4): 2026-09-25T17:12:21Z -> RELEASED (see `server3-device-leases.tsv`).
- Method: unified reference harness (`reduce_invscale_refdirect_probe`, `runner_ref.inc`
  md5 `93e6441937e9cd8394b734e124d18469`). `DEVICE_EVENT_US` primary, `HOST_WALL_US` secondary,
  **warmup=45**, 2 blocks x 31 samples, batch N=1, gap 2 s, one process, one clean attempt.
- Device d4 (ASCEND_DEVICE_ID=4), hwnput3 / cann-server3, load 1-min ~22, AICore 0%,
  HBM 90% resident (persistent VLLM, documented), no concurrent next6 probe (preflight `ps` NONE).
- Identity: Direct Parent `submission_v001_reference.asc` sha256
  `f017935d840bea5a81268d9fe137f1567df0982f4f71718f65f4672ad8da8023` (MATCH),
  Candidate `submission.asc` sha256
  `bef271b62a2c7f2d0b0ef23f5f3610129460f5a431d7d9b3dd7ac3ea9a80ad26` (MATCH, unchanged).
  No source edit. No CANNJudge submission.

## Multi-tile path confirmation (from the same-binary run, correctness D2H after timing)

| binary | shape | bad | max_abs | expected |
|---|---|---:|---:|---|
| `reduce_invscale_refdirect_probe` (Direct Parent) | 1x8192 | **2048** | 5.4174 | bad=2048 — known V001 multi-tile defect, path confirmed |

`rc=3` is the harness's `bad!=0` exit; timing block itself completed (62 samples written).
bad=2048 used only as path confirmation, per task.

## Same-binary result (Direct Parent vs itself), warmup=45

1x8192 FP32, 1 attempt, 2 blocks x 31 samples, gap 2 s, one process:

| block | n | median_us | MAD_us | MAD/med | p10 | p90 | min | max |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| B1_DEVICE | 31 | 6.60 | 0.34 | 0.0515 | 6.26 | 17.00 | 6.08 | 139.22 |
| B2_DEVICE | 31 | 19.40 | 12.88 | **0.6639** | 6.52 | 69.24 | 6.44 | 131.50 |
| ALL_DEVICE | 62 | 7.39 | 1.20 | **0.1624** | 6.42 | 66.19 | 6.08 | 139.22 |

- MAD/median (full sample) = 1.20 / 7.39 = **0.1624** — above the ≤0.10 PASS criterion,
  inside the ≤0.25 block.
- drift |B1 − B2| / ALL_median = |6.60 − 19.40| / 7.39 = **1.7321** — **> 0.25**.
- `drift > 0.25` → **`MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`** (harness/host issue, never a
  Candidate issue). Rules not loosened; shape stopped after the single clean attempt.
- **No interleaved P/C run** (runbook forbids pairs after FAIL).

Interpretation: B1 is a clean block (MAD/med 0.052, consistent with the w45 floor observed on
ALIGN), while B2 is contaminated across its whole distribution (p10 6.52 but median 19.40 —
~half the samples sit in a 14–71 µs band). The failure is a block-2 host/device interference
episode during the window (17:13 UTC, AICore snapshot 0%, HBM 90% VLLM-resident), not the
warmup systematic that w10→45 fixed: note the direction **reversed** vs the prior w10 runs
(w10: B1 12.78 > B2 6.60; w45: B1 6.60 < B2 19.40).

## Comparison with prior cycles

| attempt | warmup | window | MAD/med | drift | direction |
|---|---:|---|---:|---:|---|
| prior (sibling-contaminated) | 10 | concurrent d5/d6 | 0.2149 | 0.8667 | B1 slower |
| qb1 (quiet exclusive) | 10 | exclusive | 0.1571 | 0.8026 | B1 slower |
| **w45 (this run)** | **45** | exclusive | **0.1624** | **1.7321** | **B2 slower (reversed)** |

warmup=45 removed the systematic direction but did not stabilize block 2 on this shape.

## Files

All under this directory (`.../V002/support/results-w45-8192/`), mirrored from server3
`/home/data4t2/lelinfeng/phase4-workspaces/REDUCE-INVSCALE-X/results-w45-8192/`:

- `sb1-directparent-w45-s31-raw.tsv` — all 62 device-event samples (permanent, no deletion)
- `sb1-directparent-w45-s31-stats.txt` — per-block stats (B1/B2/ALL, DEVICE + WALL)
- `sb1.stdout.txt`, `sb1.stderr.txt`, `sb1.rc.txt`, `sb1.timestamp.txt`
- `preflight-npu-smi-usages.txt`, `preflight-proc-mem-d4.txt`, `preflight.concurrent.txt`,
  `preflight.uptime.txt`, `start.timestamp.txt`
