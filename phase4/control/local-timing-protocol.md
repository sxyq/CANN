# Local Timing Protocol (MAIN-2)

Scope: measurement infrastructure only. Do not change Candidate kernel sources, SHA, architecture, or revisions.

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

Applies to all MAIN-2 routes from the next device window onward. Measurement only — no Candidate edits.

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

- Single device per lease window via `phase4/control/server3-device-leases.tsv` (MAIN-2 rows only).
- One MAIN-2 route at a time on a device. No concurrent next6 probes.
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

Status today: **NOT_RUN** (no device timing window after stop-timing order).

Procedure (next clean window, before any Candidate pair):

1. One frozen Parent executable (or one Candidate binary used as both slots — same file).
2. SAME vs SAME, N ≥ 6 process reps × ≥11 samples, same device, same shape.
3. Record: shape, dtype, device, method (WALL_CLOCK|DEVICE_EVENT), median_us, CV, MAD, max/min, p10, p90, abs_spread_us.

Noise floor table schema (fill on first validation):

| shape | dtype | device | method | median_us | CV | MAD_us | max/min | p10 | p90 | abs_spread_us | status |
|---|---|---|---|---|---|---|---|---|---|---|---|
| TBD | TBD | TBD | TBD | | | | | | | | NOT_RUN |

Decision rule after noise floor exists:

- If same-binary itself fails CV≤0.15 / max/min≤1.30 → **do not** run Candidate pairs that session (harness or host still unstable).
- If same-binary is stable → window-qual Parent may use: absolute spread + MAD + CV **together**, not CV alone, for short kernels with naturally high relative CV.
- Interleaved pair delta remains the primary Candidate signal once the window is qualified.

## Priority after harness validation

1. SAME-BINARY harness validation (noise floor)
2. SCHED-ROWGROUP-X (STRONG_POSITIVE_LOCAL_SIGNAL retained, not promoted)
3. ALIGN-TAIL-X
4. BATCH-RESIDENT-X
5. ASYNC-TRIPLE-X
6. REDUCE-INVSCALE-X

Do not implement NEXT_CANDIDATE_HYPOTHESIS revisions (no V002+ for timing routes, no kernel edits) while Candidates remain unjudged under MEASUREMENT_BLOCKED.

## Ownership

- Protocol file and control-only commits: MAIN-2.
- MAIN-1 `cann-sixlane/*` ownership unchanged.
- UB-LIVENESS-X V003: READY_FOR_FORMAL_SUBMISSION; unified Judge Owner submits; MAIN-2 does not self-submit.
