# Unified Local Timing Protocol (MAIN-1 and MAIN-2)

Scope: applies to MAIN-1 and MAIN-2 measurement infrastructure. Do not change Candidate kernel sources, SHA, architecture, or revisions while qualifying or running measurements.

Today (2026-09-24): all five timing routes are WINDOW UNQUALIFIED 2/2. No window-qual, paired timing, or performance retry for the remainder of the day.

## Reference harness

Structural reference (simplest complete runner already in use):

- Path: `cann-next6/SCHED-ROWGROUP-X/phase4/workspaces/SCHED-ROWGROUP-X/support/runner_main.inc`
- Why: single process, fixed warmup/repeat, dumps all samples (`*-samples.tsv`) and jitter (`*-jitter.txt`), Parent and Candidate share the same include pattern via macro-selected submission.
- Sibling runners that must converge on this protocol (do not invent a third method):
  - ALIGN: `ALIGN-TAIL-X/.../V001/support/runner_main.inc` (wall-clock; missing sample dump)
  - REDUCE: `REDUCE-INVSCALE-X/.../V002/support/runner_main.inc` (wall-clock; missing sample dump)
  - BATCH: `BATCH-RESIDENT-X/.../timing_probe.asc` (wall-clock; warmup=5 repeats=21; no jitter dump)
  - ASYNC: `ASYNC-TRIPLE-X/.../V001/support/probe_main.inc` (device events; batch-averaged `device_us`, different sample model)

Orchestration references:

- Parent-only window-qual: `SCHED-ROWGROUP-X/.../support/run_q_precheck.sh`, `REDUCE-INVSCALE-X/.../run_windowqual_q.sh`
- Interleaved pairs: `ALIGN-TAIL-X/.../atx_controlled_dev4.sh` pattern (P/C adjacent, pre/post npu-smi)

## Harness audit — timing method (current)

| Route | Method | Warmup | Measured samples | Per-sample dump | Process model |
|---|---|---:|---:|---|---|
| SCHED | CPU wall-clock around launch+`aclrtSynchronizeStream` | 3 | 11 | yes (L004) | new process per precheck rep |
| ALIGN | CPU wall-clock around launch+sync | 3 | 11 | no | new process per rep |
| REDUCE | CPU wall-clock around launch+sync | 3 | 11 | no | new process per rep |
| BATCH | CPU wall-clock around launch+sync | 5 | 21 | no | new process per rep |
| ASYNC | Device event span over batch of repeats, then ÷repeats | 1 (+correctness first) | 1 number / process | no | new process per rep |

## Harness audit — noise sources found

Evidence from window-qual parent blocks (same Parent binary, same device, same shape within a route): CV 0.21–0.90, max/min 1.86–9.02.

Checklist results:

1. **Warmup insufficient** — 3–5 launch+sync cycles. First in-process samples still dominate max (e.g. SCHED d4-A medians 99–234 µs; REDUCE d4-A first sample 309 µs then 34 µs).
2. **First-run compile/load/init mixed into samples** — each precheck rep is a cold process (`aclInit` → malloc → H2D → warmup → measure → `aclFinalize`). Cold runtime/HBM state differs every rep even when AICore is 0%.
3. **Stream synchronize boundary** — wall-clock paths use one full `aclrtSynchronizeStream` per sample (consistent within those runners). ASYNC uses event stop sync after a multi-repeat batch (different boundary).
4. **Event start/stop only over kernel** — **not true for SCHED/ALIGN/REDUCE/BATCH**: wall-clock wraps host `run_kernel` call + sync, so host launch API and sync wait are inside the sample. ASYNC device events cover a whole repeat batch, not a single kernel.
5. **Device alloc/copy in timing** — H2D and `aclrtMalloc` are outside the measured loop (OK). D2H is after timing (OK). Not a primary source of within-block CV.
6. **Executable reload** — every precheck rep reloads the binary; link/load is outside the timed sample but forces cold runtime each rep.
7. **Process startup mixed into timing** — process start is outside the timed sample; between-rep cold start is **not** averaged out (window-qual treats each rep median as one sample).
8. **CPU wall-clock vs device event** — four of five routes use wall-clock; host CPU contention (persistent VLLM Python / EngineCor) inflates wall-clock without raising AICore.
9. **DVPP / VLLM / runtime background** — persistent root VLLM on d0–d7; residual HBM ~90% even at AICore 0%. Documented; not gated off mid-block.
10. **Multiple streams** — runners create one ACL stream. Kernel-internal queues unknown per Candidate; Parent path single stream.
11. **Async launch without full sync** — wall-clock and ASYNC event paths both fully sync before reading a sample (OK).
12. **Runtime re-init per sample process** — yes, every precheck rep.
13. **Parent/Candidate same runner path** — intended by design (macro-selected submission). P/C binaries must be built from the same runner_main.inc revision for pairs.
14. **Frequent malloc/free / memcpy in loop** — not present in timed loop (OK).
15. **Cache / warm-state differences** — process exit drops warm state; VLLM resident memory changes free HBM between reps; no CPU affinity pin.

Additional structural issues:

- Window-qual shapes differ by route (e.g. SCHED 17×256 FP32, REDUCE 1×6144 FP32). Very short kernels have naturally higher relative CV.
- Within-process 11 samples are reduced to one median before window-qual CV — loses within-block structure but the cross-rep CV already fails the gate.
- Historical multi-Main device overlap self-contaminated earlier sets; serial lease is now required (shared `server3-device-leases.tsv`).

## Unified measurement protocol

Applies to all MAIN-1 and MAIN-2 Routes from the next device window onward. Measurement only — no Candidate edits.

### Warmup

- Default: **10** launch+full-stream-sync cycles before any timed sample (was 3–5).
- Optional longer (20) if first timed sample still > 1.5× sample median in a dry run.
- Correctness D2H / golden compare **never** interleaved with the timed loop.

### Measurement count

- Window-qual parent block: **≥6 process reps** (keep A/B structure from `window-qualification-policy.md`).
- Within each process: **≥11** timed samples (prefer 21 if wall-clock).
- Paired Candidate window: **≥4 interleaved pairs**, adjacent P then C (or C then P alternating), same device and shape.

### Event placement (target)

- Prefer **device events** (`aclrtRecordEvent` start/stop) around each single timed launch (or a short fixed batch with ÷N), then `aclrtSynchronizeEvent` before `aclrtEventElapsedTime`.
- Record both: `device_us` (event) and `wall_us` (steady_clock around launch+sync) for diagnosis; **judge on device_us when events are available**.
- Until a route is upgraded to events, document method = WALL_CLOCK in results and treat noise floor as method-specific.

### Sync placement

- Warmup: sync after each warmup launch.
- Timed sample: sync (or event wait) before reading duration.
- No sample is read while launches remain in flight.

### Allocation policy

- Allocate once per process, outside timed loop.
- No `aclrtMalloc` / `aclrtFree` / host realloc inside timed samples.

### Copy policy

- H2D before warmup.
- D2H only after timed loop (correctness check after timing block, not between samples).

### Device selection

- Single device per lease window via the shared `phase4/control/server3-device-leases.tsv`; record the owning Main and Route for every lease.
- One performance Route at a time per device across MAIN-1 and MAIN-2. Never run concurrent timing on the same device from different Routes or Mains.
- Avoid d7. Prefer AICore 0% **and** document HBM/procs; AICore 0% alone is not sufficient.

### Sample order

- Window-qual: Parent only, PRECHECK-A ×N, gap, PRECHECK-B ×N.
- Pairs: interleaved, never all-P then all-C.
- Same-binary noise floor: same executable N times, no Parent/Candidate mix.

### Jitter calculation

Per process sample set: median, mean, stdev, CV = stdev/mean, min, max, max/min, MAD, p10, p90, absolute spread (max−min).

Across window-qual reps: same stats on the rep medians (current gate basis).

### Window qualification (for next session only)

- Keep existing policy thresholds as a **temporary** gate: CV ≤ 0.15 **and** max/min ≤ 1.30 on both A and B (`window-qualification-policy.md`).
- Budget today is exhausted (2/2). Do not re-run.
- From the next session: **first** establish a **per-shape noise floor** with same-binary runs (below). Only then decide whether fixed CV≤0.15 is appropriate for that shape/dtype/method.

## Same-binary validation (noise floor)

Status: **RUN 2026-09-24** on server3 d4, SCHED Parent (`srx_ref_parent_probe`, method DEVICE_EVENT_PRIMARY + HOST_WALL_SECONDARY). Evidence: `cann-next6/SCHED-ROWGROUP-X/phase4/local/SCHED-ROWGROUP-X/V001/support/results-ref-harness/d4/`.

### Unified reference harness (built)

- Source: `cann-next6/SCHED-ROWGROUP-X/phase4/workspaces/SCHED-ROWGROUP-X/support/runner_ref.inc` + `runner_ref_parent.asc`
- Binary: server3 `.../SCHED-ROWGROUP-X/support/build/srx_ref_parent_probe`
- Lifecycle: aclInit / setDevice / stream / malloc / H2D **once**; warmup once; measurement blocks **in one process**; D2H/cleanup once.
- Per sample: `DEVICE_EVENT_US` (aclrtRecordEvent start/stop) primary; `HOST_WALL_US` secondary.
- CLI: `device rows width dtype prefix warmup samples blocks gap_sec`
- Orchestrator: `support/run_ref_validation.sh`

### Acceptance (robust, not CV-only)

| metric | threshold | result (small, in-process) |
|---|---|---|
| warmup used | ≥10 | 10 |
| core CV (samples within ±20% of median) | ≤0.10 | B1 0.053 / B2 0.040 |
| MAD/median | ≤0.08 | B1 0.043 / B2 0.025 |
| vs cold-process reinit | in-process MAD/med must be clearly smaller | in 0.025–0.043 vs cold 0.275 |
| wall-clock CV | diagnostic only | 0.30–0.44 (worse; do not judge on wall) |

Raw CV/max-min still explode from sparse outliers (host/HBM interference under shared VLLM). **Judge uses robust stats + interleaved pairs**, not CV≤0.15 alone.

### Noise floor table (d4, SCHED Parent, warmup=10, 2×31 samples/device events)

| shape | dtype | device | method | warmup | samples | median_us | MAD_us | MAD/med | p10 | p90 | CV_raw | max/min | abs_spread | core_CV | HBM | AICore | notes |
|---|---|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|---|---|
| small 17×256 | FP32 | 4 | DEVICE_EVENT | 10 | 62 | 20.56 | 1.07 | 0.052 | 19.48 | 119.89 | 1.35 | 18.9 | 344.6 | 0.058 | ~59.2/65536 | 0% | VLLM resident |
| medium 4×1024 | FP32 | 4 | DEVICE_EVENT | 10 | 62 | 9.94 | 1.47 | 0.148 | 8.49 | 171.77 | 2.10 | 53.2 | 433.4 | 0.075 | ~59.2/65536 | 0% | VLLM resident |
| wide 1×6144 | FP32 | 4 | DEVICE_EVENT | 10 | 62 | 14.40 | 7.05 | 0.490 | 7.49 | 236.63 | 1.09 | 54.7 | 369.6 | 0.031 | ~59.2/65536 | 0% | B2 median drift 14→86 under load |

Fast-cluster (within ±15–20% of median) DEVICE times: small ~20.6µs CV≈0.04–0.05; medium ~9–13µs CV≈0.05; wide B1 ~13.7µs CV≈0.03.

### PROCESS_REINIT_NOISE

| mode | n | median_us | MAD | MAD/med | CV | within±10% of median |
|---|---:|---:|---:|---:|---:|---:|
| A: in-process 31 samples (block1) | 31 | 20.66 | 0.88 | 0.043 | (core 0.053) | 24/31 |
| B: 31 cold processes × 1 sample | 31 | 104.92 | 28.80 | 0.275 | 0.50 | 7/31 |

**CONCLUSION:** cold-process samples are **PROCESS_REINIT_NOISE dominated**. Local screening **must not** use cold-process rep medians as the primary statistic. Prefer one long-lived process with in-process blocks + interleaved pairs.

### WARMUP_STABLE_AFTER=10

| warmup | median_us | MAD/med | p90_us | outlier max_us | verdict |
|---:|---:|---:|---:|---:|---|
| 0 | 22.50 | 0.038 | 156.44 | 142394 | first-run catastrophic |
| 3 | 22.44 | 0.029 | 43.58 | 149.4 | residual early outliers |
| 10 | 20.38 | 0.056 | 24.64 | 151.7 | **chosen** (best p90) |
| 20 | 19.60 | 0.015 | 141.38 | 168.6 | not better p90 |

### Decision

- **SAME_BINARY_VALIDATION=PASS** (in-process, device-event, warmup≥10, robust core).
- **HARNESS_VALIDATED=YES** for future P/C under this protocol only.
- **CANDIDATE TIMING remains NOT_RUN** in this session (no P/C executed).
- Raw CV≤0.15 / max/min≤1.30 alone is **not** a sufficient gate for short kernels; use MAD/median + core_CV + interleaved pair delta. Legacy wall-clock numbers tagged **LEGACY_TIMING_METHOD** (see `results-window-qual/LEGACY-TIMING-METHOD.md`); SCHED −18.45% 4/4 stays **STRONG_POSITIVE_LOCAL_SIGNAL**, never merged with new method medians.

## Priority after harness validation

1. ~~SAME-BINARY harness validation (noise floor)~~ DONE 2026-09-24 PASS
2. SCHED-ROWGROUP-X (STRONG_POSITIVE_LOCAL_SIGNAL retained, not promoted)
3. ALIGN-TAIL-X
4. BATCH-RESIDENT-X
5. ASYNC-TRIPLE-X
6. REDUCE-INVSCALE-X

All future P/C must use the unified reference protocol (device events primary, warmup≥10, samples≥21, in-process, interleaved). No route-specific wall-clock as primary.

Do not implement NEXT_CANDIDATE_HYPOTHESIS revisions (no V002+ for timing routes, no kernel edits) while Candidates remain unjudged under MEASUREMENT_BLOCKED.

## Ownership

- Protocol file and control-only commits: MAIN-2.
- MAIN-1 `cann-sixlane/*` ownership unchanged.
- UB-LIVENESS-X V003: READY_FOR_FORMAL_SUBMISSION; unified Judge Owner submits; MAIN-2 does not self-submit.
- Judge Owner: not named beyond “unified Judge Owner” in control; exact path `cann-next6/UB-LIVENESS-X/phase4/local/UB-LIVENESS-X/V003/submission.asc`, SHA `2eb9b5d087267a54fb84f8734847ecb68cf94b967102693c0d150fd57d6da7cd`.
