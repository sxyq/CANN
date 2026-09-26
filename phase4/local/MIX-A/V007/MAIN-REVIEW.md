# MIX-A V007 Main Review

Date: 2026-09-25

## Decision

No local performance verdict has been formed. Keep V007 unchanged. The retained contaminated measurements do not support a performance conclusion, promotion, rejection, or Online submission. `BUILD=PASS`; `CORRECTNESS=PASS`; `EXECUTABLE_IDENTITY=PASS`; `READY_FOR_SAME_BINARY=true`. `TIMING=MEASUREMENT_BLOCKED` is the timing-stage state only; the allowed d4-d6 devices currently have heavy VLLM HBM use, and d7 is excluded from performance timing.

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
- The runner validates the latest status for each lease ID, rejects an active conflicting device lease, and disallows d7 for timing. Host-only lease tests passed, including release followed by a later valid lease on the same device.
- Existing targeted V007 correctness passed for the `rows=1`, `D=256` FP32, FP16, and BF16 cases; both variants produced identical reference outputs.
- The unified runner has not been executed. No current-protocol same-binary qualification or new paired sample exists.

## Measurement status

- Four earlier Parent/Candidate observations remain `LOAD_CONTAMINATED`, with mixed directions; they cannot establish gain or regression.
- The old wall-clock runner does not meet the current local timing procedure. Use only the unified runner after Main grants a current exclusive lease.
- Qualify the exact Direct Parent and shape with same-binary blocks first. Run interleaved V003/V007 pairs only if that qualification passes.

## Next action

Keep V007 and its source identity unchanged. Wait for a permitted device window and suitable load, then run same-binary qualification followed by paired measurement if allowed. Do not create V008 before Main returns a decision.
