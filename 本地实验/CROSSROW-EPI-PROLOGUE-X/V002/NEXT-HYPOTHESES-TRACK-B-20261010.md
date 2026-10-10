# W5-R01 route-local next-hypothesis review — 2026-10-10

AGENT_ID: W5-R01/Zeno
ROUTE: CROSSROW-EPI-PROLOGUE-X
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
SOURCE_SCOPE: own R1 V001/V002 declarations, kernels, runner and Local evidence only; no other Route source/worktree was read
BASE: V002 result commit `03e3069f03cfb70e4ecd64296e3c0c7abb8651a6`

## Evidence boundary

- V001's early result had a Parent/Candidate label-identity problem; the corrected-label retry is retained but target-device contaminated. Its incompatible raw outcomes are not a stable mechanism verdict.
- V002 source delta is one issue-point relocation for the existing successor-row x/residual MTE2 prologue. The latest device-3 raw is a real 21-pair run with no target-NPU process, but Parent CV=25.939%, Candidate CV=49.534%, and no separate same-binary qualification. Verdict remains `NEEDS_ONE_MORE_LOCAL`; the source mechanism is neither accepted nor technically falsified.
- User-supplied cross-route review excludes the proposed MTE3 output-drain/Store-epilogue axis as overlapping R31A V021 and STORE-EPILOGUE-X. Main-1 Sync/Fastpath and W3 CROSSROW are also excluded by current task direction. No corresponding private source was inspected.

## Candidate hypotheses assessed (maximum three)

### A. Partial retained-row cache for the 2-tile FP16 cached-row path

MECHANISM: In R1 `Candidate.asc`, `valueFp32Buf_` is allocated for `kCacheElems=8192` FP32 values (32 KiB), while the target row has 6144 values. A targeted variant could retain only tile 0's x+residual FP32 values and recompute tile 1 from x/residual during pass 2, reducing that buffer to one 4096-element tile (16 KiB) while leaving gamma/bias caching and V002's successor prefetch unchanged.
EXPECTED_SHAPE: FP16 `rows=16,width=6144,blocks=8`, two rows per effective block and two 4096-element tiles.
WHY_IT_MIGHT_HELP: Lower per-block UB footprint could help if the retained row, rather than arithmetic, is limiting execution resources.
FAILURE_MODE: Re-reading 2048 x/residual elements and repeating half add/widening for tile 1 can cost more than the smaller UB footprint saves; a lifetime or tile-index error can corrupt correctness.
MINIMAL_OFAT: Candidate-only change to the FP16 generic cached-row value-retention rule; preserve H1 load placement, parameter caching, reduction order, stores, ABI and all event boundaries; run existing paired runner correctness on target plus M=1, then one paired Local.
EXISTING_RUNNER: YES, target shape and correctness/local modes are accepted.
DEDUP: Distinct from V002 H1's MTE2 successor-load issue point and from the user-identified Main-1 MTE3/Store-drain axis. The R1 declaration says W3 V012 changed pass-2 parameter double buffering, not this buffer; however, R1-only evidence cannot establish `MECHANISM_OVERLAP=NO` against the broader W3 CROSSROW row-retention mechanism. Since the user explicitly excludes W3 CROSSROW, this candidate is **NOT CLEARED FOR CODE**. No patch exists; patch-similarity percentage is not applicable.

### B. One-row-per-block ownership for the target

MECHANISM: Current `rows=16,blocks=8` produces `localRows=2`; using 16 blocks would give one row per block and more independent blocks, at the cost of disabling the successor guard and per-core parameter reuse.
EXPECTED_SHAPE: FP16 `rows=16,width=6144`, blocks 8 versus 16.
WHY_IT_MIGHT_HELP: More concurrent row blocks and less serial work per block.
FAILURE_MODE: The one-row path reloads parameters and cannot use the cross-row prefetch; those extra loads may dominate. This also changes dispatch/path predicates, so it would not isolate V002 H1.
MINIMAL_OFAT: The existing runner passes one global `--blocks` value to both P and C and rejects Local when `rows/effectiveBlocks < 2` (runner_main.cpp lines 413–419). Thus the current runner cannot perform this candidate comparison; changing the runner guard or adding an alternate mode would expand measurement infrastructure and is out of scope. Not runner-qualified; no code change.
DEDUP: Scheduling/ownership is distinct from H1 and the specifically excluded MTE3 drain, but no cross-route `MECHANISM_OVERLAP=NO` claim is made. Not eligible under the current runner and scope.

### C. FP16 cached-row MTE3 output drain

STATUS: REJECTED_BEFORE_CODE. User-provided review identifies substantive overlap with Main-1 R31A V021's delayed output MTE3-to-V wait and proximity to STORE-EPILOGUE-X writeback/drain. `MECHANISM_OVERLAP=YES`; do not implement or create V003 from this idea.

## Route conclusion and Planning direction request

No candidate above currently satisfies all three required conditions: distinct mechanism under the stated W3/Main-1 exclusions, explicit code-level OFAT, and isolation by the existing paired runner. Therefore no next Revision is created. This is **not a technical falsification of V002 H1**: the latest V002 Local is too dispersed to accept or reject H1, and earlier V001/V002 attempts have identity/load issues or instability. The factual route state is: H1 not promoted; no stable Local Best; no orthogonal, runner-qualified next code change established from R1-local evidence.

PLANNING_DIRECTION_REQUEST: identify or authorize a search axis outside successor-row MTE2 issue placement, W3 CROSSROW row-retention/pipeline, Main-1 Sync/Fastpath/scalar handoff, and Main-1 MTE3/Store-epilogue drain; alternatively provide a public metadata-only dedup ruling on candidate A. This is a direction request, not a request to wait for routine per-version approval. Until a mechanism is cleared, remain on the same R1 context and do not manufacture V003.

GIT_SYNC: V002 result commit `03e3069f03cfb70e4ecd64296e3c0c7abb8651a6` is safe locally. Two bounded HTTPS push commands were launched: `timeout 25s git push origin research/w5-r01-crossrow-epi-prologue` and `timeout 30s git push origin research/w5-r01-crossrow-epi-prologue`; each returned rc=124 with no server response/output. The verification command `timeout 15s git ls-remote origin refs/heads/research/w5-r01-crossrow-epi-prologue` also returned rc=124 with no SHA. These timeouts leave the remote outcome ambiguous; no network command remains active and no retry will be made. The local remote-tracking ref is stale at `d2412ab5cb2d091679947c65d90d45912541b1a5`; `REMOTE_SHA=UNVERIFIED`.
