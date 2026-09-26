# Consolidation Before Second Six-Lane (2026-09-24)

No kernel source, candidate implementation, server3 run, or CANNJudge submission was performed by Main.

## CANONICAL

- Branch: `exp/independent-breadth`
- Remote: `origin/exp/independent-breadth`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann`

## WORKTREE INVENTORY

| Path | Branch | HEAD relation | Classification |
|---|---|---|---|
| `cann` | `exp/independent-breadth` | canonical | KEEP |
| `cann-sixlane/R31B` | `exec/sixlane-20260923-r31b` | same HEAD, dirty local evidence | ACTIVE_KEEP |
| `cann-sixlane/R31A` | `exec/sixlane-20260923-r31a` | same HEAD, dirty local evidence | ACTIVE_KEEP |
| `cann-sixlane/MIX-A` | `exec/sixlane-20260923-mix-a` | same HEAD, dirty V007 | ACTIVE_KEEP |
| `cann-sixlane/WIDE-X-FRESH4` | `exec/sixlane-20260923-wide-x-fresh4` | same HEAD, untracked workspace | ACTIVE_KEEP |
| `cann-sixlane/MODE-X-R015C` | `exec/sixlane-20260923-mode-x-r015c` | same HEAD, untracked workspace | ACTIVE_KEEP |
| `cann-sixlane/EXT-ASCEND-X` | `exec/sixlane-20260923-ext-ascend-x` | same HEAD, untracked local | ACTIVE_KEEP |

`cann-worktrees/` is an empty container, not a Git worktree.

No first-round worktree met DELETE_SAFE criteria (all six remain scheduler ACTIVE).

## EVIDENCE PRESERVED INTO CANONICAL

Copied from ACTIVE worktrees without mutating those worktrees:

- `phase4/local/MIX-A/V007/**`
- `phase4/local/R31A/V019/**`
- `phase4/local/R31B/V016/**`
- `phase4/local/EXT-ASCEND-X/V001/**`
- `phase4/workspaces/MIX-A/MIX-A-V007-*` and compile logs
- `phase4/workspaces/R31A/R31A-V019-*` and adapters
- snapshots under `phase4/archive/active-sixlane-20260924/`

Restored champion evidence:

- `phase4/online/R31B/V011/submission.asc` (was working-tree deleted; identical to `submission.txt`)

## HISTORICAL BRANCH CONTENT ARCHIVE

- 49 non-merged local branches considered
- 47 branches had unique paths missing from canonical and from `归档/phase3-before-reset-20260920/`
- 402 files exported to `phase4/archive/historical-branches-20260924/<branch>/`
- Summary: `phase4/archive/historical-branches-20260924/summary.json`

Those historical branch commits are still not ancestors of canonical; they are retained (not force-deleted).

## CLASSIFICATION SUMMARY

- MERGE_TO_CANONICAL_THEN_DELETE: none among first-round worktrees (zero unique commits; ACTIVE)
- ARCHIVE_TO_CANONICAL: historical branch unique trees + active worktree evidence snapshots
- DELETE_SAFE worktrees: none
- ACTIVE_KEEP: all six `cann-sixlane/*` worktrees
- DIRTY_UNRESOLVED: none blocking second round; canonical dirty PARKED candidates remain untracked by design

## SECOND SIX-LANE CREATED

Container: `/Users/sunyiyang/Desktop/Project/cann-next6/` (not a worktree)

| Route | Branch | Brief |
|---|---|---|
| ASYNC-TRIPLE-X | exp/next6-async-triple-x | ROUTE-BRIEF.md |
| BATCH-RESIDENT-X | exp/next6-batch-resident-x | ROUTE-BRIEF.md |
| SCHED-ROWGROUP-X | exp/next6-sched-rowgroup-x | ROUTE-BRIEF.md |
| REDUCE-INVSCALE-X | exp/next6-reduce-invscale-x | ROUTE-BRIEF.md |
| ALIGN-TAIL-X | exp/next6-align-tail-x | ROUTE-BRIEF.md |
| UB-LIVENESS-X | exp/next6-ub-liveness-x | ROUTE-BRIEF.md |

All six based on canonical HEAD `ae46d7c`.

## NEXT6 MAIN REVIEWS

| route | rev | audit | correctness | local | decision |
|---|---|---|---|---|---|
| ALIGN-TAIL-X | V001 | PASS | PASS pair_max_abs=0 | LOAD_CONTAMINATED 4 probes | NEEDS_ONE_MORE_LOCAL |
| BATCH-RESIDENT-X | V001 | PASS | PASS batch path; col-split parent-inherited | LOAD_CONTAMINATED inconsistent | NEEDS_ONE_MORE_LOCAL |
| SCHED-ROWGROUP-X | V001 | PASS | PASS | LOAD_CONTAMINATED; aligned control also moved | NEEDS_ONE_MORE_LOCAL |
| REDUCE-INVSCALE-X | V001 | PASS | FAIL D>6144 parent partial-align | PARTIAL_CONTAMINATION | LOCAL_REJECTED; assign V002 correctness-only |
| ALIGN-TAIL-X | V001 | PASS | PASS pair_max_abs=0 | LOAD_CONTAMINATED | NEEDS_ONE_MORE_LOCAL |
| SCHED-ROWGROUP-X | V001 r2 | PASS | PASS | set1/set2 disagree; aligned control opposite | NEEDS_ONE_MORE_LOCAL; require VLLM stop |
| ASYNC-TRIPLE-X | V001 | - | - | worker general-1 UnknownError | replacement worker same route |

### NEXT6 batch review after first full handoffs

| route | rev | correctness | local | decision |
|---|---|---|---|---|
| ASYNC-TRIPLE-X | V001 | PASS | contaminated noise>signal | NEEDS_ONE_MORE_LOCAL + PROBES PARKED |
| BATCH-RESIDENT-X | V001 | PASS batch path | no clean window | NEEDS_ONE_MORE_LOCAL + PROBES PARKED |
| SCHED-ROWGROUP-X | V001 | PASS | set1/set2 disagree; no VLLM-free | NEEDS_ONE_MORE_LOCAL + PROBE PARKED |
| REDUCE-INVSCALE-X | V002 | PASS D<=32768 | repair cost below noise | correctness ACCEPTED; R019 perf deferred; PROBES PARKED; align hypothesis refuted; sync-only approved |
| ALIGN-TAIL-X | V001 | PASS | no clean window 81min | NEEDS_ONE_MORE_LOCAL + PROBES PARKED |
| UB-LIVENESS-X | V001 | FAIL | N/A | LOCAL_REJECTED; assign V002 correctness-only |

Shared blocker: persistent root/zhangkaijie VLLM + HBM saturation; agents cannot create clean window alone.
| UB-LIVENESS-X | V002 | partial improve still FAIL | raw invalid | LOCAL_REJECTED; assign V003 pass1 acc fix |
| UB-LIVENESS-X | V003 | PASS bad=0 full battery | alias probes inconclusive | ONLINE_CANDIDATE frozen to pool; Judge Owner only |

## Server3 8-NPU window survey (2026-09-24)

| dev | free HBM MB | used/65536 | AICore | resident | role |
|---:|---:|---|---:|---|---|
| 0 | 5580 | 59956 | 31% | VLLMWorker_TP | busy compute |
| 1 | 5530 | 60006 | 32% | VLLMWorker_TP | busy compute |
| 2 | 5587 | 59949 | 32% | VLLMWorker_TP | busy compute |
| 3 | 5585 | 59951 | 32% | VLLMWorker_TP | busy compute |
| 4 | **6349** | 59187 | **0%** | VLLMEngineCor 55664MB | **PRIMARY run window** |
| 5 | 5658 | 59878 | **0%** | VLLMWorker_TP | SECONDARY run window |
| 6 | 5661 | 59875 | **0%** | VLLMWorker_TP | SECONDARY run window |
| 7 | 9 | 65527 | 17% | python 61828MB | AVOID |

Policy update: compile/link is CPU (no HBM need) and may parallelize on server3. NPU correctness + paired probes pin to **ASCEND_DEVICE_ID=4** primary; 5/6 allowed as secondary if no concurrent next6 probe. Residual VLLM HBM documented — do not wait for full VLLM stop. Device 7 forbidden.
| ALIGN-TAIL-X | V001 dev4 | PASS pairs | DIR 2/4 noise 393pp concurrent | NEEDS_ONE_MORE_LOCAL; serialize dev4 |
| BATCH-RESIDENT-X | V001 set2 | PASS batch | DIR 3/4 median -23.9% concurrent ALIGN | NEEDS_ONE_MORE_LOCAL; serialize dev4 |
| ASYNC-TRIPLE-X | V001 set2 | PASS | DIR 3/6 noise CV41% concurrent reduce | NEEDS_ONE_MORE_LOCAL; exclusive later |
| SCHED-ROWGROUP-X | V001 set3 | PASS | DEV4 residual | NEEDS_ONE_MORE_LOCAL; exclusive later |
| REDUCE-INVSCALE-X | V002 recheck | PASS | repair-cost PARK; concurrent srx on pair1 historically | R019 DEFERRED; d4 free |
| BATCH-RESIDENT-X | set2 | PASS | DIR 3/4 median -23.9% concurrent | **EXCLUSIVE_DEV4 GRANTED for set3** |
| ALIGN-TAIL-X | L002 controlled | PASS | MODERATE DIR3/4 median+19% slower | NEEDS_ONE_MORE_LOCAL; d4 free |
| BATCH-RESIDENT-X | exclusive set3 | PASS | exclusive interleaved; DIR 3/4 pair3 +185% | NEEDS_ONE_MORE_LOCAL (prior exclusive) |
| SCHED-ROWGROUP-X | L004 controlled | PASS | LOAD_CONTAMINATED sofm0.305 DIR0.5 | NEEDS_ONE_MORE_LOCAL; d4 free |
| ASYNC-TRIPLE-X | L006 controlled | PASS path triple | LOAD_CONTAMINATED CV29% DIR3/6 | NEEDS_ONE_MORE_LOCAL; d4 free |
| REDUCE-INVSCALE-X | L005 controlled | PASS both D6144 | LOAD_CONTAMINATED jitter78% | NEEDS_ONE_MORE_LOCAL; d4 free |
| ALIGN-TAIL-X | R2 baseline | N/A candidate | parent UNSTABLE d4/d5/d6 CV0.27-0.32 | MEASUREMENT_BLOCKED + NEEDS_ONE_MORE_LOCAL |
| BATCH-RESIDENT-X | R2 baseline | N/A candidate | parent UNSTABLE d4/5/6 | MEASUREMENT_BLOCKED + NEEDS_ONE_MORE_LOCAL; pair3 OUTLIER |
| ASYNC-TRIPLE-X | R2 baseline | N/A candidate | parent UNSTABLE d4/5/6 | MEASUREMENT_BLOCKED + NEEDS_ONE_MORE_LOCAL |
| REDUCE-INVSCALE-X | R2 baseline | SHA P!=C verified | parent UNSTABLE d4/5/6 candidate0 | MEASUREMENT_BLOCKED + NEEDS_ONE_MORE_LOCAL |
| SCHED-ROWGROUP-X | R2 pairs d6 | PASS | baseline STABLE d6; DIR 4/4 -18.45%; parent sofm0.558 | MEASUREMENT_BLOCKED + NEEDS_ONE_MORE_LOCAL; not REJECT |
| ROUND2_SUMMARY | five routes | - | all parent/jitter gates block ONLINE judgment | MEASUREMENT_BLOCKED x5; no ONLINE from R2; UB unchanged WAIT JUDGE |
| SCHED-ROWGROUP-X | window qual | - | d6+d4 FAIL CV/maxmin 2/2 | WINDOW_UNQUALIFIED; NEEDS_ONE_MORE_LOCAL+MEASUREMENT_BLOCKED; STRONG_POSITIVE retained |
| ALIGN-TAIL-X | window qual | - | d6+d4 FAIL 2/2 | WINDOW_UNQUALIFIED; NEEDS_ONE_MORE_LOCAL+MEASUREMENT_BLOCKED |
| BATCH-RESIDENT-X | window qual | - | d6+d4 FAIL 2/2 | WINDOW_UNQUALIFIED; NEEDS_ONE_MORE_LOCAL+MEASUREMENT_BLOCKED |
| ASYNC-TRIPLE-X | window qual | - | d6+d4 FAIL 2/2 | WINDOW_UNQUALIFIED; NEEDS_ONE_MORE_LOCAL+MEASUREMENT_BLOCKED |
| REDUCE-INVSCALE-X | window qual | P/C SHA OK | d6+d4 FAIL 2/2 candidate0 independent | WINDOW_UNQUALIFIED; NEEDS_ONE_MORE_LOCAL+MEASUREMENT_BLOCKED |
| WINDOW_QUAL_SUMMARY | five routes | - | all 2/2 UNQUALIFIED CV<=0.15 maxmin<=1.30 | stop timing today; SERVER_RESOURCE_BLOCKED; UB JUDGE_READY |

## Harness audit + timing protocol (2026-09-24 afternoon)

No Candidate source, no new revision, no timing retry, no CANNJudge submit.

- UB V003 package re-verified: submission.asc SHA256 matches sidecar `2eb9b5d0…`, source-meta DECISION=ONLINE_CANDIDATE / AUDIT=PASS / CORRECTNESS=PASS, diff.patch 14578 B, pool row + judge-handoff present, `phase4/online/UB-LIVENESS-X/` absent. Status remains READY_FOR_FORMAL_SUBMISSION for unified Judge Owner only.
- Measurement audit across five runners: primary method is CPU wall-clock around launch+sync (SCHED/ALIGN/REDUCE/BATCH); ASYNC uses device events over a repeat batch. Warmup only 3–5; each window-qual rep is a cold process (aclInit/finalize). Alloc/H2D outside timed loop (OK). Host launch + sync wait sit inside wall-clock samples. Persistent VLLM + ~90% HBM on d4/d6 remains. Observed parent CV 0.21–0.90 and max/min up to 9.0 within same binary/device/shape.
- Reference harness designated: SCHED `support/runner_main.inc` (sample+jitter dump). Unified rules in `phase4/control/local-timing-protocol.md`.
- Same-binary noise floor: NOT_RUN today. Next clean window validates SAME vs SAME before any Candidate pair; then SCHED → ALIGN → BATCH → ASYNC → REDUCE.
- Five routes stay NEEDS_ONE_MORE_LOCAL + MEASUREMENT_BLOCKED (SERVER_RESOURCE_BLOCKED), attempts 2/2. SCHED keeps STRONG_POSITIVE_LOCAL_SIGNAL BUT LOAD_NOT_QUALIFIED.

## Unified harness SAME-BINARY validation (2026-09-24 evening)

- CONTROL HEAD checked SYNCED with origin at start (b82afb6).
- Built `runner_ref.inc` / `srx_ref_parent_probe` (device event primary, one-time init, in-process multi-block). Deployed to server3 d4 lease HARNESS-REF.
- SAME-BINARY (SCHED Parent, 17×256 FP32, warmup10, 2×31): PASS on robust core (MAD/med ≤0.043, core CV ≤0.058). Raw CV high from sparse outliers; wall-clock worse (CV 0.30–0.44).
- PROCESS_REINIT: cold 31×1 MAD/med 0.275 vs in-process 0.043 — cold-process primary stats banned.
- WARMUP_STABLE_AFTER=10 (warmup0 catastrophic first samples).
- Noise floor recorded for small/medium/wide in local-timing-protocol.md. Legacy wall-clock tagged LEGACY_TIMING_METHOD; SCHED −18.45% remains STRONG_POSITIVE_LOCAL_SIGNAL.
- SAME_BINARY_VALIDATION=PASS; HARNESS_VALIDATED=YES; CANDIDATE TIMING still NOT_RUN this session.
- UB V003 reconfirmed READY_FOR_FORMAL_SUBMISSION; package unchanged; MAIN-2 no self-submit.

## Corrected harness status + SCHED shape revalidation (2026-09-25)

- HARNESS_VALIDATED corrected to PARTIAL_SHAPE_CONDITIONAL (small PASS / medium NEEDS / wide NEEDS-FAIL). Outlier policy fixed in local-timing-protocol.md before any new P/C.
- SCHED exact-shape same-binary (Direct Parent, d4, warmup10, 2×31 device events): 33×100 PASS, 17×256 PASS; 7×65 and 17×257 NEEDS_VALIDATION (7×65 → MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE). Repeat-batch N=10 not adopted (drift/MAD worse).
- SCHED interleaved P/C only on PASS shapes: 17×256 median delta −0.76% within floor; 33×100 −31.81% with reverse pair + high C MAD → decision NEEDS_ONE_MORE_LOCAL (not ONLINE, not REJECT). V001 SHA 0fae0a42 unchanged; no new revision.
- ALIGN/BATCH/ASYNC/REDUCE: still NEEDS_ONE_MORE_LOCAL + MEASUREMENT_BLOCKED; each needs its own exact-shape same-binary PASS before P/C. Not run this turn.
- UB V003: ONLINE_CANDIDATE, JUDGE_READY=YES, READY_FOR_FORMAL_SUBMISSION, JUDGE_OWNER_REQUIRED; no self-submit.
- NEW_OFFICIAL_SCORE: none.

## Long-horizon exploration + route arbitration (2026-09-25)

- RESEARCH_PUSH_SYNCED=YES: LOCAL_HEAD=REMOTE_HEAD=d450a2c, AHEAD=0, BEHIND=0 (pushed 21a34c9 contract §R + d450a2c six research handoffs).
- Contract: execution-contract.md gained one-time section R Long-Horizon Parallel Exploration (two-track model, 3–5 hypotheses/cycle, handoff stop conditions, no micro-revision loops).
- Six TRACK-B handoffs landed at phase4/research/<ROUTE>/next-hypotheses.md (5 hypotheses each, 1553 lines total).
- ROUTE ARBITRATION: parameter staging/gamma-bias residency/row-batch param reuse → BATCH-RESIDENT-X (UB-H2 DUPLICATE·DEFER_TO_BATCH); MTE2 ingress queue/prefetch/copy-compute-store pipeline → ASYNC-TRIPLE-X (UB-H3 DEFER_TO_ASYNC); reduction topology/partial reduction/reduction temps → REDUCE-INVSCALE-X (UB-H5 DEFER_TO_REDUCE); row-group ownership/rows-task/core-fill/shape scheduling → SCHED-ROWGROUP-X (ALIGN-H2 DEFER_TO_SCHED). UB keeps only buffer lifetime/aliasing/peak UB footprint/live-set budgeting; ALIGN keeps only DataCopy/DataCopyPad/aligned bulk/tail/copy-direction asymmetry.
- APPROVED NEXT backlogs (not immediate revisions): SCHED H1 core-fill (H2 second, H3 needs evidence) — forbidden while V001 undecided; ALIGN H1 threshold bulk+tail — forbidden while V001 undecided; BATCH H1 two-stage row-group compute — must shrink to one conceptual mechanism at declaration; ASYNC H1 inter-pass prologue prefetch — only after current Candidate disposed, no queue-depth/buffer adds; REDUCE H2 first-output-tile MTE2 overlap (H1 rsqrt stays in feasibility research, H3 donor evidence); UB H1 TRUE_LIVE_SET_BUDGET — no V004 before Judge return.
- PROBE SHAPE CHANGES (measurement design only): ASYNC retires 8×1024 as primary (width=1024 → tileCount=1 → Candidate≡SEED); new primary width>1024, tileCount≥2 preferably ≥3 (4096/6144/8192). REDUCE single-tile 1×6144 no longer primary reduction evidence; add MULTI_TILE_D probe (D>6144) verified multi-tile for both binaries.
- UB judge handoff: JUDGE_OWNER=MAIN-1 designated; MAIN-2 handoff-only, no self-submit. SUBMISSION_ID/REMOTE_SHA/OFFICIAL_SCORE/DECISION all PENDING.
- Track-A next window priority: SCHED 33×100 re-run (exact Parent+Candidate, unified device events, same-binary PASS first, interleaved P/C, two independent blocks). ALIGN awaits 2×100 same-binary. BATCH harness migration to unified device-event protocol pending (legacy wall-clock data non-decision-grade).
- NEW_OFFICIAL_SCORE: none. local deltas (-31.81%, -18.45%, -0.76%) are LOCAL SIGNAL ONLY.

## SCHED 33×100 Track-A re-qualification (2026-09-25, lease LH-SCHED-33x100)

- Lease LH-SCHED-33x100 on d4: LEASED 2026-09-25T09:40:00Z, RELEASED 2026-09-25T09:55:27Z; only that one lease line appended.
- Identity verified before run: V001 submission SHA 0fae0a42 unchanged, Direct Parent SHA 62de32df unchanged; parent/v001 probe md5 956821cc/41a9aae7 match; runner_ref.* md5 match worktree. No source edits.
- Host: load ~23, VLLM resident since Sep 22 (d0–d3 busy), d4 AICore 0%, HBM 59186/65536 (~90%).
- Same-binary (Direct Parent vs itself, 33×100 FP32, warmup 10, 2×31 device events, batch N=1), 2 attempts allowed, 2 used:
  - attempt1: ALL MAD/med 0.096, drift |B1−B2|/med 0.200 → NEEDS_VALIDATION (B1 slow tail 78–137 µs).
  - attempt2: ALL MAD/med 0.032, drift 0.104 → NEEDS_VALIDATION (misses drift threshold by 0.004; sparse B1 outliers p90 160 µs).
  - Both below 0.25 on MAD/med and drift → not MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE.
- Interleaved P/C NOT run (same-binary PASS is a precondition). No Candidate timing this window.
- Decision: **NEEDS_ONE_MORE_LOCAL** (not ONLINE_CANDIDATE, not LOCAL_REJECTED). Prior 33×100 floor (MAD/med 0.049, drift 0.062) did not reproduce under current host load.
- Evidence: `cann-next6/SCHED-ROWGROUP-X/phase4/local/SCHED-ROWGROUP-X/V001/support/results-pc-33x100-lh/` (raw `sb33-attempt{1,2}-raw.tsv`, stats, npu-smi snapshots, summary.json, SUMMARY.md).
- NEW_OFFICIAL_SCORE: none. Local stats only.

## BATCH harness migration to unified device-event protocol (2026-09-25)

BATCH-RESIDENT-X's local timing harness was migrated from the legacy wall-clock probe (`timing_probe.asc`, warmup 5, median-only output) to the unified reference runner ported from the SCHED implementation: new `phase4/workspaces/BATCH-RESIDENT-X/support/` (`runner_ref.inc` + `runner_ref_batch.asc` + CMake target `brx_ref_probe` + `run_ref_smoke.sh`) gives one aclInit/malloc/H2D/stream per process, warmup once (10), ≥21 in-process samples with DEVICE_EVENT_US primary and HOST_WALL_US secondary, per-sample `*-raw.tsv` dump with jitter stats, and the 10/11-arg CLI (optional batch_n, default 1); the golden check after the timed loop reuses BATCH's own `npu_correctness.cpp` model. Built on server3 and functionally smoked on d5 (exit 0, 21 device-event samples, `bad=0`) with no timing lease claimed and no performance numbers recorded or judged — evidence under `cann-next6/BATCH-RESIDENT-X/phase4/local/BATCH-RESIDENT-X/harness/`, migration notes in `HARNESS-MIGRATION-20260925.md`, prior wall-clock summaries tagged LEGACY_TIMING_METHOD with data retained. Candidate `submission_v001.asc` SHA ad961c5837971bf989c9989b822cc2719a6a285c60969d35e1530463b7c8721d verified unchanged; no kernel edit, no new Revision, no P/C timing this turn.


## Long-horizon cycle 2026-09-25 evening — six-route parallel Track-A+B (MAIN-2)

All six route agents completed. Candidate SHAs unchanged on every route. No CANNJudge submit. No kernel source edits.

### Track-A results

| route | probe | same-binary | P/C | decision |
|---|---|---|---|---|
| SCHED-ROWGROUP-X V001 | 33x100 + 17x256 FP32 d4 | PASS both (MAD/med 0.054/0.026; drift 0.099/0.025) | 33x100 −51.68/−51.07/−52.04/−50.99 median **−51.38%** 4/4 no reverse; 17x256 control ≈ neutral ex-p1 −0.2% | **ONLINE_CANDIDATE** (local; Judge Owner submit only) |
| BATCH-RESIDENT-X V001 | 100x256 FP32 d5 | PASS (MAD/med 0.032/0.028; drift 0.035) | 4 pairs −4.34/+0.25/+1.36/−4.73 median −4.53% 2/4 INCONSISTENT within cross-process swing max/min 1.10 | NOT_QUALIFIED_THIS_WINDOW / NEEDS_ONE_MORE_LOCAL |
| ALIGN-TAIL-X V001 | 2x100 FP32 d4 | FAIL 2/2 (MAD/med 0.220/0.117; drift 0.372/0.426) | not run | MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE |
| ASYNC-TRIPLE-X V001 | 8x8192 + 8x4096 d4 | FAIL both (MAD/med 0.457/0.345) | not run | MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE |
| REDUCE-INVSCALE-X V002 | 1x8192 FP32 d6 | FAIL (MAD/med 0.215; drift 0.867) | not run | MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE |
| UB-LIVENESS-X V003 | — | package re-verified | — | ONLINE_CANDIDATE frozen; Judge still PENDING |

### Track-B research (all six, appended to next-hypotheses.md)

- SCHED: H1 core-fill READY (rowGroup=2 correction on 33x100 → ~16 tasks); H2 even-split READY; H9 core-scaling sweep READY (measured 1→3 cores = 1.95×, ~65% efficiency); H6/H7 NEEDS_MORE_EVIDENCE; 6 candidates screened out.
- BATCH: H1 OFAT-split into S1 fused epilogue (READY, recommended first), S2 vector rstd (depends on S1), S3 ablation; H2 param stripe NEEDS_MORE_EVIDENCE (WIDE-X ownership question); H3 blockFactor overlaps EXT-ASCEND-X (PARKED).
- ALIGN: H1 bulk+tail READY (blocked on measurement); H3 copy-direction NEEDS_MORE_EVIDENCE; **H6 new** sizeAligned without startAligned (parent+V001 shared; 2x4100/2x4103 falsification).
- ASYNC: H1 inter-pass prologue prefetch READY (single-digit µs expected; latency/issue-chain bound at 1.75MB/launch vs 55–70µs core time); H2/H3 NEEDS_MORE_EVIDENCE; H4/H5 parked; fill/steady/drain model for tileCount=8 recorded.
- REDUCE: H1 rsqrt 16-shape precision battery designed (NOT run); H2 first-output-tile overlap READY; H3 wider ReduceSum → SCREENED_LOW_VALUE (FP32 slack +573 elems only); OPT-4 infeasible FP32.
- UB: H1 TRUE_LIVE_SET_BUDGET OFAT plan + G1–G4 static gates complete; tile-budget matrix 3 dtype × 7 D; **latent V003 overrun** FP32 D∈(15352,16384] old EstBytes 188416 vs actual 204864 > 192KB.

### Main dispositions

1. SCHED V001: accept ONLINE_CANDIDATE. Frozen SHA `0fae0a42…` appended to online-candidate-pool.tsv. Unified Judge Owner (MAIN-1) submits only. No H1/H9 implementation until formal result + NEXT_HYPOTHESIS.
2. BATCH V001: NEEDS_ONE_MORE_LOCAL. After disposition, first approved slice = H1-S1 fused epilogue only.
3. ALIGN V001: MEASUREMENT_BLOCKED. H6 may get one correctness-only device slot (no timing). H1 waits on measurement path.
4. ASYNC V001: MEASUREMENT_BLOCKED. Next window must be exclusive (no sibling leases). H1 ready after disposition.
5. REDUCE V002: MEASUREMENT_BLOCKED. Parent multi-tile signature bad=2048 confirmed. H2 after disposition; H1 rsqrt battery only in a correctness-only window.
6. UB V003: remain frozen / JUDGE_READY. H1 plan approved as next research product; V004 still forbidden until Judge returns.
7. Device policy update: concurrent multi-route timing on d4/d5/d6 in the same minutes destroyed floors (ASYNC/REDUCE/ALIGN). **From next cycle, one performance Route across all devices at a time** for same-binary/P/C; Track-B stays parallel.

### Files written by Main this turn

- phase4/control/online-candidate-pool.tsv (SCHED V001 row)
- phase4/control/server3-device-leases.tsv (late LH3-SCHED-33x100 pair)
- this consolidation section

## Exclusive quiet-window requal + Main decision (2026-09-25 night)

Serialized same-binary retries on d4 (QR-ALIGN-2x100 → QR-ASYNC-8x8192 → QR-REDUCE-8192), zero concurrent sibling leases, host load 22–27 from resident VLLM + other users.

| shape | prior (concurrent) | exclusive retry | verdict |
|---|---|---|---|
| ALIGN 2x100 | MAD/med 0.22/0.12 drift 0.37/0.43 | MAD/med 0.286 drift 0.381 | MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE |
| ASYNC 8x8192 | 0.457 / 0.443 | 0.272 / **0.040** | MEASUREMENT_PROTOCOL_BLOCKED (drift fixed by exclusive; MAD still >0.25) |
| ASYNC 8x4096 | 0.345 / 0.089 | 0.416 / 0.748 | MEASUREMENT_PROTOCOL_BLOCKED |
| REDUCE 1x8192 | 0.215 / 0.867 | 0.157 / 0.803 | MEASUREMENT_PROTOCOL_BLOCKED (parent multi-tile bad=2048 confirmed again) |

Correction to earlier attribution: concurrent sibling leases explain ASYNC block-drift only. ALIGN and REDUCE reproduce a systematic B1 > B2 bias even in exclusive quiet windows on ~7–110µs short kernels → harness/host layer (resident VLLM + foreign CPU load), not Candidate and not sibling contention.

### Main decision
1. ALIGN / ASYNC / REDUCE remain MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE. No P/C. No more identical same-binary retries that spend a lease without changing the measurement layer.
2. Do not change Candidate kernels. Do not change the pre-registered outlier policy after seeing results.
3. Open a measurement-layer investigation (read-only + optional same-binary-only harness experiments): why first block is systematically slower; whether longer steady-state, block-order randomization, or a quieter host window can produce a valid floor. Any protocol edit still requires same-binary validation before P/C use.
4. SCHED V001 stays ONLINE_CANDIDATE pending Judge Owner. BATCH stays NEEDS_ONE_MORE_LOCAL. UB V003 stays frozen / JUDGE_READY.
5. Track-B continues on all six routes; measurement blockage must not idle the research track.

Evidence: phase4/control/handoff-quiet-retry-requal.md; per-route results-quiet-retry/.

## Main decisions after Track-B deepening (2026-09-25)

1. **BATCH H2 ownership**: PARAMETER RESIDENCY / stripe-resident params stay with **BATCH-RESIDENT-X** per fixed route scope (PARAMETER RESIDENCY → BATCH). Historical WIDE-X idea-pool row is donor history, not active ownership. H2 is NOT DEFER_TO_WIDE-X.
2. **ALIGN H6**: authorize one **correctness-only** device slot (no timing) for the pre-registered battery (2×4100 FP32, 2×4103 FP32, isolators 1×4100 FP32 / 2×4104 FP16 / 1×4104 FP16; 10 runs or 3-run minimal). If the 5-branch tree reports a real unaligned-start failure, Main will decide correctness-first repair scope including parent surface — not a performance revision.
3. **ASYNC**: no dedicated window for H2. Future quiet window target floor MAD/med ≤0.03 then H1 first + H3 piggyback only.
4. **REDUCE H1 rsqrt build**: still gated on V002 disposal + NEXT_HYPOTHESIS. Control-half of the battery may run in a correctness-only slot before that if a device is free.
5. **Measurement resolution note** (all routes): protocol floor 0.10 on 55–70µs kernels is ~5.5–7µs, larger than current hypothesis ceilings (order-1µs). Null P/C results on ordinary floors are unmeasurable, not refuted.
6. SCHED H1/H9 remain blocked pending Judge / NEXT_HYPOTHESIS.

## ALIGN H6 closed (2026-09-25)

Correctness battery on d4 (10 runs × 5 shapes × parent/V001 = 100, all bad=0):
2×4100 FP32, 2×4103 FP32, 1×4100 FP32, 2×4104 FP16 isolator, 1×4104 FP16 control.
Branch taken: REJECTED_AS_VIOLATION — unaligned GM start is not a dav-2201 correctness edge; official DataCopy text needs only dtype-byte alignment on GM.

Main disposition:
- Close H6. No parent correctness repair. `&& startAligned` remains optional hardening only.
- Five shapes are correctness-clean → eligible for future P/C once the measurement layer unblocks.
- Evidence: cann-next6/ALIGN-TAIL-X/phase4/local/ALIGN-TAIL-X/V001/support/results-h6-correctness/

## Measurement-layer root cause + protocol warmup update (2026-09-25)

Research report: phase4/control/measurement-layer-b1b2-research-20260925.md
Independent corroboration: phase4/research/measurement-b1b2-20260925/ (keep).

Ranked causes:
1. warmup=10 too short — device/driver settles at ~40–45 launches; B1 of short kernels stays elevated. 20/25 historical runs show B1 slower. warmup≥30–50 yields PASS-grade floors on ALIGN 2x100 and REDUCE 1x8192.
2. post-gap rep1 cold-wake spike (optional gap=0 removes it; MAD robust either way).
3. co-tenant VLLM bursty interference (dominant residual for ASYNC; needs idle AICore window).
4. event window absorbs host delay (background; not the B1 elevation).

Main action taken:
- local-timing-protocol.md Warmup default **10 → 45**. Criterion unchanged (MAD/med ≤0.10 and drift ≤0.10). Per-route same-binary re-validation required before any new P/C.
- Do not loosen the outlier/shape criterion.
- ASYNC waits for a genuinely idle VLLM window (all-NPU AICore ~0%); do not spend more identical quiet-retry leases under current load.
- Lease MR-B1B2-20260925 released.

Next Track-A under warmup=45: ALIGN 2x100 then REDUCE 1x8192 same-binary → if PASS, interleaved P/C same window. BATCH may also re-run 100x256 P/C if time (prior 2/4 was within cross-process swing).

## warmup=45 Track-A cycle (2026-09-25 evening)

| route | shape | same-binary w45 | P/C | decision |
|---|---|---|---|---|
| ALIGN-TAIL-X V001 | 2x100 FP32 | PASS 0.0543/0.0946 (was 0.286/0.381 at w10) | 4 pairs −69.7*/−3.23/−4.81/+14.51; ex-polluted median −4.02% 3/4 | NEEDS_ONE_MORE_LOCAL |
| REDUCE-INVSCALE-X V002 | 1x8192 FP32 | FAIL 0.1624/1.7321 (B2 contaminated; opposite direction vs w10) | not run | MEASUREMENT_BLOCKED |
| BATCH-RESIDENT-X V001 | 100x256 FP32 | Parent PASS 0.0383/0.0120; Candidate self 0.4953/0.4334 on d4 | 4 pairs C cells polluted (+39%~+130% from bad cells) | MEASUREMENT_BLOCKED (not LOCAL_REJECTED) |

Warmup fix validated on ALIGN. Remaining failures are intermittent co-tenant contamination on d4 (host load 24→34 during window), not warmup residual. Same BATCH candidate was clean on d5/w10 earlier.

Main dispositions:
- ALIGN: another interleaved P/C window later (need ≥4 clean pairs); H1 remains READY but waits V001 disposal.
- REDUCE / BATCH: retry only in a new quieter window or other device; do not treat polluted cells as performance verdicts.
- ASYNC: still needs idle VLLM window.
- SCHED V001 / UB V003: still with Judge Owner (MAIN-1). No self-submit.
- Criterion still not loosened.

Evidence: results-w45-2x100/, results-w45-8192/, results-w45-100x256/; HANDOFF-TRACK-A-W45-20260925.md.
