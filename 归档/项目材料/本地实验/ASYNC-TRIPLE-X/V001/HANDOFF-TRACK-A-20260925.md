# ASYNC-TRIPLE-X — Track-A Handoff (2026-09-25, lease TA-8x8192)

```text
SHA CANDIDATE   2defc6c270e898c00aa151dbef00f68c72ccf77f33e54db18c75b8bf4f4b4f9c UNCHANGED
                (no kernel edits this turn; measurement layer only)
SHA PARENT      f20da79c7086483c7cf0bddad630bdea92a219992a5f061c9fd3a6b8edbc3572 (SEED, same-binary floor binary)
DEVICE          cann-server3 d4 (primary, leased 14:27:51Z, released 14:39:28Z).
                d5 = BATCH, d6 = REDUCE concurrent siblings same window; d7 untouched.
DECISION        NEEDS_ONE_MORE_LOCAL + MEASUREMENT_BLOCKED (same-binary window
                UNQUALIFIED on both sanctioned shapes -> MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE)
P/C PAIRS       NOT RUN (pair runner conditional on floor PASS; aborted by design)
OFFICIAL_SCORE  none produced; Local % != Official Score
```

## What ran

Unified device-event protocol (`runner_ref.inc`, byte-identical SHA 89f8380a to SCHED/ALIGN
copies): warmup=10, 2 blocks x 31 samples, batch_n=1, in-process, device events primary /
host wall secondary, raw `*-raw.tsv` preserved.

- Ref probes built on server3 (`build-ref/async_ref_seed_probe`, `async_ref_v001_probe`) from
  unmodified SEED/V001 sources — build only, no Candidate touch.
- Correctness smoke @8x8192, both binaries: `bad=0`, `max_abs=2.6226e-06`.
  Floor runs (SEED): 8x8192 `bad=0`; 8x4096 `bad=0`, `max_abs=2.38419e-06`.

## Same-binary floors (Direct Parent SEED; pre-registered rule: PASS iff MAD/med <= 0.10
## AND block drift <= 0.10 AND bad=0)

| shape | tileCount | ALL MAD/med | B1 MAD/med | B2 MAD/med | drift | verdict |
|---|---:|---:|---:|---:|---:|---|
| 8x8192 (PRIMARY) | 8 | 0.4565 | 0.5106 | 0.2591 | 0.4427 | FAIL |
| 8x4096 (fallback) | 4 | 0.3449 | 0.4719 | 0.1849 | 0.0893 | FAIL |

Both MAD/med > 0.25 -> `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` per
`phase4/control/local-timing-protocol.md` — not a Route failure, no mechanism judged.

## Contamination context (why the window failed)

- Sample streams bimodal: fast cluster ~55-70 us (kernel) vs ~190-230 us clusters
  (host-driven); clusters last 5-10 samples then switch — external, not per-sample jitter.
- During the primary floor (14:34Z) sibling MAIN-2 routes were running: REDUCE 1x8192
  floor on d6, BATCH pairs on d5; host load ~23; VLLM resident (documented).
- Sibling outcomes in the same window: REDUCE 1x8192 UNQUALIFIED (MAD/med 0.2149,
  drift 0.8667), ALIGN 2x100 FAIL (drift 0.372/0.426), BATCH 100x256 floor PASS but
  pairs within noise. Only BATCH's floor passed today.
- Structural note: `run_kernel` performs per-launch tiling `aclrtMalloc`+H2D sync inside
  the event span (host gap inflates device_us). Same for P and C — paired deltas stay
  valid once a window qualifies — but it makes the floor sensitive to host contention.

## Evidence (all raw preserved)

- `support/results-ref-8x8192/{floor/,smoke-*}` — primary floor raw/stats/npu-smi/protocol/verdict
- `support/results-ref-8x4096/floor/` — fallback floor, same set
  (label corrections recorded inside protocol.txt / floor-verdict.json; raw tsv untouched)
- Workspaces: `workspaces/ASYNC-TRIPLE-X/{runner_ref.inc,runner_ref_seed.asc,
  runner_ref_v001.asc,CMakeLists.ref.txt,build_ref_probes.sh,run_ref_floor.sh,run_ref_pairs.sh}`
- Lease: `phase4/control/server3-device-leases.tsv` TA-8x8192 LEASED/RELEASED
- OLD-PROBE-8x1024-NON-QUALIFYING tag unchanged; no 8x1024 data used.

## Residual

1. Requalify the same-binary floor (prefer 8x8192) in a window with NO active sibling
   route leases, then run the 4-pair P/C already scripted in `run_ref_pairs.sh`.
2. V001 correctness smoke at 8x4096 not separately run (8x8192 smoke covers both binaries;
   both shapes' SEED floors bad=0).
3. Track-B continued in parallel — see `phase4/research/ASYNC-TRIPLE-X/next-hypotheses.md`
   (fill/steady/drain model + H1 OFAT deepening appended this turn).
