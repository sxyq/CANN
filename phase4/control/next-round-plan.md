# Next Round Plan (after Hardening + Full Review)

## Gate

Do **not** start new performance experiments until:

1. Submission Script Hardening — DONE (`脚本/cannjudge-submit.mjs`)
2. Full Project Review — DONE (`full-project-review.md`)
3. Local vs Online Calibration baseline — DONE (`local-online-calibration-review.md`)
4. Champion Bottleneck Review — DONE (`champion-gap-analysis.md`)
5. Route Classification Fixed — DONE (this document)

## Route tree (authoritative)

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

R31A and R31B are **EXPLOIT**. They must **not** appear under `4 RECOMMENDED EXPLORATION DIRECTIONS`.

## EXPLOIT ×2

### R31B

- R31B-V011 · 15/15 · 45.16 · Overall Champion
- CONTEXT_CLASS: HISTORICAL_EXPLOIT
- Next: Champion review → one hypothesis → Local → Online → Promote / Reject

### R31A

- R31A-V016 · 15/15 · 45.00 · Route Best
- CONTEXT_CLASS: HISTORICAL_EXPLOIT
- Next: new independent branch from **V016** only → one hypothesis → Local → Online → Promote / Reject
- Do **not** stack changes on V017 (44.45) regression

## 4 Recommended Exploration Directions (EXPLORE only)

Exactly four exploration slots. No R31A / R31B.

| # | Route | Start parent | CONTEXT_CLASS | status | next |
|---|---|---|---|---|---|
| 1 | **MIX-A** | V003 (44.69) | HYBRID / MULTI_CHANGE | EXPLORE online Best | isolate one hybrid hypothesis → clean Parent → Local → Online |
| 2 | **WIDE-X-FRESH4** | none (fresh) | FRESH_BLIND | EXPLORE COMPILE_FAIL (`sqrtf`, size_t narrowing) | same Route Agent: build fix → correctness → paired local → Online Candidate / Reject |
| 3 | **MODE-X-R015C** | none | HISTORICAL_DERIVED | EXPLORE BUILD_PASS | NPU correctness → paired local → Online Candidate / Reject |
| 4 | **EPI-X-FRESH** | none | FRESH_BLIND | EXPLORE BUILD_PASS + CORRECTNESS_FAIL | Review → RETAIN / PARK; if RETAIN: correctness-only fix → local test; else PARK / REPLACE |

Main does not edit Kernel sources. WIDE build fix belongs to the WIDE Route Agent.

## Process contract (unchanged)

- One conceptual change per revision.
- Record ROUTE / REVISION / DIRECT_PARENT / PARENT_SOURCE_SHA / PARENT_SCORE / SINGLE_HYPOTHESIS / CONTEXT_CLASS before edit.
- Local-first: compile → correctness → paired local → online only if promising.
- PROMOTE only if Pass and Official > Direct Parent.
- Formal submit only via `npm run cannjudge:submit -- --yes --source <file>` with SHA identity checks.
- Main does not edit kernel sources.

Park / no new assignment: A001, G001, H001, I001, B001–F001 (archived/parked per scheduler).

## Evidence before any ONLINE_CANDIDATE

- `submission.sha256` sidecar match
- local compile PASS
- local correctness PASS
- paired local latency (or explicit `NO_COMPARABLE_LOCAL_DATA` → not promising)
- proxy quality not `MISLEADING`
- formal submit with `LOCAL_SHA == REMOTE_SHA`

## Explicitly out of scope this round

- New kernel revisions
- Creating 4 exploration agents
- CANNJudge submissions
- Using R31A-V017 single-probe as online gate
- Clipboard / paste formal submits
