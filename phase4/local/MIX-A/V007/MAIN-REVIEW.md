# MIX-A V007 Main Review

Date: 2026-09-25

## Decision

No local performance verdict has been formed. Keep V007 unchanged. The retained contaminated measurements do not support a performance conclusion, promotion, rejection, or Online submission. `BUILD=PASS`; `CORRECTNESS=PASS`; `EXECUTABLE_IDENTITY=PASS`; `READY_FOR_SAME_BINARY=true`. `TIMING=MEASUREMENT_BLOCKED` is the timing-stage state only. Any server3 device with live HBM usage below 100% and no conflicting shared lease is eligible; AICore and resident processes are recorded but do not independently stop timing.

## Lineage and source review

- Route: `MIX-A`
- Revision: `V007`
- Direct Parent: `MIX-A-V003`
- Parent Official Score: `44.69`
- Parent source SHA-256: `1a1857a945ce1f04e3877437882b21a3d5071dddcd697103d7b539a0feb5c706`
- Candidate source SHA-256: `a63ad29a997ae2fe8a1238c1a47a9d5ddfb14d16f725fca975ce5c55523f28eb`
- Context: `HYBRID`
- Hypothesis: remove the pre-load `SyncVToMTE2()` in `ProcessNarrowMidFast` when the existing dispatch selects exactly one local row.
- `SINGLE_CHANGE_AUDIT=PASS`. The executable source diff removes that synchronization call only. Dispatch, math, copy order, and other dtype paths remain unchanged.
- Candidate bytes match the retained source and sidecar.

## Build and correctness evidence

- The retained V007 route build and link passed on CANN `8.5.0.alpha002`, Ascend910B3, `dav-2201`.
- The unified Parent/Candidate runner compiled and linked on server3 against the declared V003 and V007 sources. Runner executable SHA-256: `dce996af2ec709219819de3e9ba908f0d41744f2b9820965a1385e080b949110`.
- The runner validates the latest status for each lease ID and rejects an active conflicting device lease. Host-only lease tests cover d0-d7, including release followed by a later valid lease on the same device.
- Existing targeted V007 correctness passed for the `rows=1`, `D=256` FP32, FP16, and BF16 cases; both variants produced identical reference outputs.
- The unified runner has not been executed. No current-protocol same-binary qualification or new paired sample exists.

## Measurement status

- Four earlier Parent/Candidate observations remain `LOAD_CONTAMINATED`, with mixed directions; they cannot establish gain or regression.
- The old wall-clock runner does not meet the current local timing procedure. Use only the unified runner after Main grants a current exclusive lease.
- Qualify the exact Direct Parent and shape with same-binary blocks first. Run interleaved V003/V007 pairs only if that qualification passes.

## Next action

Keep V007 and its source identity unchanged. Wait for a permitted device window and suitable load, then run same-binary qualification followed by paired measurement if allowed. Do not create V008 before Main returns a decision.

## Current Protocol Run: 2026-09-27

- Exact source and existing Parent/Candidate/unified-runner identities were reused. No build, link, correctness, or Candidate source work was repeated.
- Device 7 lease: `M1-MIX-A-V007-D7-R2-20260927T065041Z`; shape `rows=1,width=256,dtype=FP32`; warmup `45`; `21` interleaved device-event pairs.
- Parent same-binary qualification: `PASS`; block MAD/median `0.040252` and `0.029987`; full-set `0.037700`; block drift `0.035783`; Parent correctness bad `0`.
- P/C correctness: Parent bad `0`, Candidate bad `0`, pair output difference `0`.
- Candidate device-event MAD/median was `0.294315`. Pair deltas had `11` faster and `10` slower pairs; median paired delta was `-0.939%` (`-0.04 us`). The direction and Candidate dispersion do not establish improvement beyond the shape noise floor.
- Device record: HBM `3431-3432/65536 MB` (`5%`) before/after; AICore is recorded only, including a postflight reading of `100%`; no process ran on d7. HBM admission passed, and AICore did not stop the run.
- Raw evidence: `support/results/m1-mix-a-v007-d7-r2-paired-20260927T065041Z.samples.tsv`, `support/results/m1-mix-a-v007-d7-r2-paired-20260927T065041Z.summary.tsv`, and `support/results/m1-mix-a-v007-d7-r2-20260927T065041Z-load.txt`.

Current disposition: `BUILD=PASS`; `CORRECTNESS=PASS`; `SAME_BINARY=PASS`; `TIMING=READY`; `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`; `LOCAL_BEST=UNCHANGED`; `ONLINE_WORTHY=NO`. V007 remains the current Candidate. No V008 and no Online submission.
