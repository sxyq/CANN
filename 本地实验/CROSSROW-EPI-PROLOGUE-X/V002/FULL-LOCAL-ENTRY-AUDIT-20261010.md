# W5-R01 full Local Score entry audit — 2026-10-10

AGENT_ID: W5-R01/Zeno
ROUTE: CROSSROW-EPI-PROLOGUE-X
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
CURRENT_HEAD_AT_AUDIT: 28fb00d89c9ecbe7f7a01d253bf9fb2a343ea6a1
SCOPE: read-only review of formally published Git history and current public scripts; no other Route private source/worktree read

## Decision

FULL_15_CASE_LOCAL_ENTRY_AVAILABLE_IN_CURRENT_R1_WORKTREE: NO
EXACT_15_CASE_REPRODUCIBLE: NO (published Main-2 reconciliation says `PARTIAL public metadata only`)
V003_ELIGIBLE: NO
V002_CHANGED: NO
NEW_RUNNER: NO

The current R1 worktree contains no complete Local Score matrix, no exact 15-case raw measurement vector, and no runnable published Main-2 full-suite entry. Single-shape V001/V002 values remain shape-local evidence only and cannot be converted into a full score.

## Historical published entries located

### Historical unified suite runner

Published history object: commit `c80d259a9`, path `工具/main2-unified-local-suite.sh`.

Default inputs and parameters in that historical script:

```text
SUITE=$ROOT/研究/主代理/MAIN-2/MAIN2-UNIFIED-LOCAL-SUITE-V1.tsv
MANIFEST=$ROOT/研究/主代理/MAIN-2/MAIN2-OFFICIAL-CALIBRATION-SET.tsv
DEVICE=7
WARMUP=20
SAMPLES=11
RUNS=3
RUN_ROOT=$ROOT/研究/主代理/MAIN-2/calibration-v2/runs/<UTC timestamp>
```

The historical command form is:

```text
SUITE=<published suite.tsv> MANIFEST=<published calibration-set.tsv> DEVICE=<NPU> WARMUP=<n> SAMPLES=<n> RUNS=3 RUN_ROOT=<run-root> ONLY=<ALL|route/revision> bash 工具/main2-unified-local-suite.sh
```

Its binary call shape is:

```text
<probe> <device> <rows> <width> <dtype_code> <evidence_prefix> <warmup> <samples> 1 0
```

Its output files are `run-config.tsv`, `measurements.tsv`, `raw-samples.tsv`, `version-status.tsv`, per-case build/correctness/log/raw evidence, and pre/post `npu-smi` snapshots. The published V1 suite is **16 cases C01–C16**, not 15; the case rows are preserved in the historical object `研究/主代理/MAIN-2/MAIN2-UNIFIED-LOCAL-SUITE-V1.tsv` at `c80d259a9`.

This historical runner is not a current R1 entry: the script, V1 suite, calibration manifest, and its support/package assumptions are absent from the current R1 worktree. It also uses historical template/source packages and a different harness contract, so it cannot be silently adopted as the existing R1 runner.

### Later canonical scorer

Published history object: commit `5549f6bf6`, paths `工具/main2-canonical-scorer.mjs`, `工具/main2-canonical-harness/MANIFEST.tsv`, `工具/main2-canonical-harness/*`, and `本地实验/CANONICAL-LOCAL-SUITE-V1.tsv`.

The canonical scorer is not a 15-case score. Its frozen suite is seven cases:

```text
C12  8x8192    BF16  CORE
C13  128x16384 FP16  CORE
C14  128x16384 BF16  CORE
C16  9x32768   BF16  CORE
C01  1x128     FP16  DIAGNOSTIC
C08  8x2048    BF16  DIAGNOSTIC
C11  1x8192    FP32  DIAGNOSTIC
```

Its validation contract requires device-event timing, `warmup=45`, `samples_per_block=31`, three unique valid runs, source SHA identity, harness SHA identity, and valid raw samples. It computes the scored vector from the four CORE cases only:

```text
score = 100 * geometric_mean(anchor_median_us / candidate_median_us)
```

The diagnostic cases do not enter the canonical score. Historical invocation is `node 工具/main2-canonical-scorer.mjs anchor|scoreboard`, but the referenced assets/registry/run roots are not present in the current R1 worktree and the command was not run.

### Official formula, not a Local replacement

The published Judge helper `工具/cannjudge-submit.mjs` contains the Official per-case formula:

```text
s_i = 100 / (1 + log_1.5(time_i / best_time_i))
total = mean(s_i)
```

That helper operates on Judge/API result rows and does not supply the missing R1 15-case local raw vector. It cannot be used to turn V001/V002 single-shape raw into a full Local Score.

## Missing inputs for an admissible R1 full-score run

The following are missing from the current R1 worktree and were not recreated:

1. A current published 15-case shape/tbest manifest with exact case IDs, rows, width, dtype, expected path, and reference values. The historical V1 suite is 16 cases; the canonical later suite is 7 cases.
2. A current R1-compatible existing harness entry that consumes that manifest without copying another Route's private source or creating a second Runner.
3. Full Parent and Candidate raw/event outputs for every required case, with exact source/ABI/artifact/Runner identity, qualification, load snapshots, and aggregation input.
4. A formally selected aggregation contract connecting the full case vector to a Local score. Published alternatives are the 16-case historical runner outputs, the 7-case canonical geometric score, and the Judge/API Official mean; none is a current R1 15-case Local contract.

## Route state and next action

CURRENT_LOCAL_SCOPE: `SHAPE_LOCAL_ONLY`
FULL_LOCAL_SCORE: `NOT_COMPUTABLE`
V002_STATUS: `NEEDS_ONE_MORE_LOCAL / NO_PROMOTION`; existing raw and identity evidence remain unchanged.

NEXT_ACTION: Main-2 performs read-only review of this audit and Planning decides whether to publish/authorize one current, R1-compatible full matrix and aggregation contract, or to issue a new mechanism/route decision. Until that decision, do not create V003, do not run the historical scripts, do not copy another Route's private source, and do not treat any single-shape value as a full score.
