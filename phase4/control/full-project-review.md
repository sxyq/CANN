# Full Project Review (2026-09-23)

## Scope

- Workspace: `Codex with ChatGPT · cann` → `/Users/sunyiyang/Desktop/Project/cann`
- Branch: `exp/independent-breadth` @ `466da2cebd8cd64c4587a1795fac41a67cc3e109`
- `origin/HEAD` unset; remote `origin = https://github.com/sxyq/CANN.git`
- No `git reset --hard` / `git clean -fd`. Dirty candidates preserved.
- No new performance experiments this round.

## Workspace tree (current)

| path | role | state |
|---|---|---|
| `phase4/README.md` | CURRENT rules | updated this round |
| `phase4/control/` | CURRENT control plane | results/scheduler/champions/calibration |
| `phase4/review/` | CURRENT server3 + logs | 7 historical Bests PASS compile/link |
| `phase4/online/` | CURRENT online evidence | per-route result snapshots |
| `phase4/local/` | CURRENT local-only records | MIX-A/MODE/EPI/WIDE slots |
| `phase4/champions/` | CURRENT Best index | `current.tsv` |
| `phase4/archive/` | HISTORICAL_ONLY | retired routes; do not rewrite |
| `phase4/workspaces/` | CURRENT sources + dirty candidates | keep |
| `脚本/` | CURRENT submit/poll tooling | hardened this round |
| `归档/` | HISTORICAL_ONLY | phase3 freeze; do not rewrite |

Dirty / untracked candidates (preserve):

- modified: `phase4/workspaces/R31B/CMakeLists.txt`, `WIDE-X/local_types.h`, `WIDE-X/submission.asc`
- untracked: EPI-X, EPI-X-FRESH, MODE-X, MODE-X-R015C, MIX-A local experiment files, R31A-V018, R31B-V015, WIDE-X-FRESH*, V016 investigation probes

## Route Best reconfirm (from `phase4/online/**/result.json`, not version number alone)

| Route | Best rev | pass | Official | source sha prefix | note |
|---|---|---|---|---|---|
| OVERALL | R31B | V011 | 15/15 | 45.16 | `a8c19a19…` |
| R31A | V016 | 15/15 | 45.00 | `dd130938…` | V017=44.45 REJECT |
| R31B | V011 | 15/15 | 45.16 | `a8c19a19…` | champion |
| MIX-A | V003 | 15/15 | 44.69 | `1a1857a9…` | V004+ local-only / not promoted online |
| A001 | V017 | 15/15 | 36.41 | `c906bc45…` | parked; idea donor |
| H001 | V008 | 15/15 | 29.04 | `ec3a631d…` | parked |
| I001 | V004 | 15/15 | 15.53 | `1ef42c72…` | parked |
| G001 | V002 | 11/15 | — | `f5fc7a57…` | no Pass revision; best by passCount |

Handoff reference matched for all rows above.

## Server3 reconfirm (`phase4/review/server3-best-repro.tsv`)

| item | evidence | status |
|---|---|---|
| R31B-V011 | device/submission/full_link PASS | REPRODUCIBLE |
| R31A-V016 | device/submission/full_link PASS | REPRODUCIBLE |
| MIX-A-V003 | device/submission/full_link PASS | REPRODUCIBLE |
| A001-V017 | device/submission/full_link PASS | REPRODUCIBLE |
| G001-V002 | device/submission/full_link PASS | REPRODUCIBLE |
| H001-V008 | device/submission/full_link PASS | REPRODUCIBLE |
| I001-V004 | device/submission/full_link PASS | REPRODUCIBLE |
| WIDE-X-FRESH4 | device/submission/full_link FAIL (`sqrtf` in aicore) | COMPILE_FAIL |
| MODE-X-R015C | device/submission/full_link PASS | REPRODUCIBLE (host probes only) |
| EPI-X-FRESH | device/submission/full_link PASS | REPRODUCIBLE (correctness FAIL) |

Environment: CANN `8.5.0.alpha002`, Ascend910B3, `dav-2201`.

## Document classification

Scope: project-produced docs under `phase4/`, `脚本/`, root `README.md` (exclude `归档/`, `.agents/skills`, node_modules).

### CURRENT (must reflect latest rules)

| file | note |
|---|---|
| `README.md` | phase boundary |
| `phase4/README.md` | rules + hardening + incident (updated) |
| `脚本/README.md` | formal submit contract (updated) |
| `phase4/champions/current.tsv` | Best index; matches result.json |
| `phase4/control/results.tsv` | online outcomes ledger |
| `phase4/control/scheduler.tsv` | slot ownership / status |
| `phase4/control/judge-payload-provenance-audit.md` | provenance; `ab5bb308` now resolved to remote kernel of 6ab35deb |
| `phase4/control/local-online-calibration.tsv` | V017 MISLEADING row + this-round expansion |
| `phase4/control/online-candidate-pool.tsv` | V017 rejected |
| `phase4/control/idea-pool-29-routes.md` | next-round idea source |
| `phase4/control/architecture-evidence-map.md` | NEW this round |
| `phase4/control/champion-gap-analysis.md` | NEW this round |
| `phase4/control/full-project-review.md` | this file |
| `phase4/control/next-round-plan.md` | NEW this round |
| `phase4/review/server3-best-repro.tsv` | current server3 truth |
| `phase4/online/*/result.json` | online truth |

### PARTIALLY_STALE

| file | stale part | keep |
|---|---|---|
| `phase4/control/architecture-plan.md` | Round1 A001–F001 only | historical design snapshot; superseded by scheduler slots |
| `phase4/control/online-candidate-pool.tsv` | only R31A-V017 row | structure OK; needs more paired rows |
| `phase4/local/*/local-result.json` | missing latency / device numbers | status still useful |
| `phase4/workspaces/*/README.md` | some route handoffs | local knowledge |
| `phase4/control/r31-*.tsv` | exploratory pressure tables | evidence for R31 only |

### STALE (do not treat as authority)

| file | why |
|---|---|
| any note claiming `6ab35deb` CE diagnostic MISSING / `ab5bb308` UNRESOLVED **without** the later probe JSON | superseded by `submission-json-probe.json` + `root-cause-note.md` |
| implicit “clipboard submit OK” wording outside hardened README | disabled |
| G001 “score 9.69” without labeling official vs calculated | official empty; only calculated |

### HISTORICAL_ONLY (do not rewrite)

- `归档/phase3-before-reset-20260920/**`
- `phase4/archive/retired-routes/**` (A001/G001/H001/I001 handoffs)
- Round1 A001–F001 narratives once superseded (architecture-plan remains as snapshot)

### CONFLICTING (resolve in favor of listed truth)

| conflict | resolution |
|---|---|
| version number vs score (R31A V017 > V016 by version) | score: V016 45.00 is Best |
| G001 latest V009 vs V002 | Best = V002 by passCount 11/15 |
| compile PASS ≠ local score | local-online-calibration + local-result decision fields |
| `result.json source.sha256` vs judge payload | SUBMIT_CLIENT_HASH vs TRANSFORMED_VERIFIED (provenance audit) |
| WIDE-X-FRESH4 local-result “NOT_RECORDED” vs server3 FAIL | server3 log wins for compile |

## R31A-V017 required check

| field | value |
|---|---|
| Parent | R31A-V016 = 45.00 |
| V017 | Pass 15/15, Official **44.45** |
| local probe | D=24576 rows=128, 4.480s → 4.286s, **-4.33%** |
| online delta | 45.00 → 44.45 (**-0.55**) |
| classification | **Proxy = MISLEADING** |

Do not use this single probe as online decision evidence.

## Route classification (fixed 2026-09-23)

```text
PROJECT
│
├── EXPLOIT ×2
│   ├── R31B-V011 45.16
│   └── R31A-V016 45.00
│
└── EXPLORE ×4
    ├── MIX-A-V003 44.69
    ├── WIDE-X-FRESH4
    ├── MODE-X-R015C
    └── EPI-X-FRESH
```

**4 Recommended Exploration Directions** (EXPLORE only — no R31A/R31B):

1. MIX-A
2. WIDE-X-FRESH4
3. MODE-X-R015C
4. EPI-X-FRESH

## Next

The route classification above records the state at the time of this review. See the current six-slot state in `phase4/control/next-round-plan.md` and `phase4/control/scheduler.tsv`.

## Current Slot State Addendum

The active lineup is now R31B and R31A as the two Exploit routes, plus MIX-A, WIDE-X-FRESH4, MODE-X-R015C, and EXT-ASCEND-X as the four Explore routes. EXT-ASCEND-X is ACTIVE / NOT_STARTED. EPI-X-FRESH is PARKED; its source and local result remain retained.

This is a state synchronization only. No Agent, isolated worktree, branch, or context was created in this turn. The historical results and local evidence above are unchanged.
