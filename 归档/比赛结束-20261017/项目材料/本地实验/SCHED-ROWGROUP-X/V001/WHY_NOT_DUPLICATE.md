# WHY_NOT_DUPLICATE — SCHED-ROWGROUP-X V001

Route tests one combination: R016 shape-aware rows/task scheduling **plus** R012 32-byte-safe row-group ownership.

## Vs R001–R029 (idea pool)

| ID | idea | why not this route |
|----|------|--------------------|
| R001 | two-pass sum then apply | different pass structure; not scheduling ownership |
| R002 | resident-y | data residency, not row-group ownership |
| R003 | pure Ascend C direct | compile style only |
| R004 | low-precision middle | dtype change |
| R005 | large tile | tile size, not ownership |
| R006 | chunked reduce | reduction redesign |
| R007 | ReduceSum API | reduction primitive |
| R008 | tile across cores / D-slice | splits D; this route owns whole rows/groups |
| R009 | DataCopyPad tail | copy primitive only |
| R010 | manual tail | epilogue/mask |
| R011 | manual vector reduce | reduction |
| R012 | 32B row-group alone | **donor half only**; idea-pool marks R012 alone as covered/widespread. V001 is R016 baseline + this donor, not R012 alone |
| R013 | double-buffer pipeline | pipeline; explicitly out of V001 scope |
| R014 | GammaBias residency | parameter cache; out of scope |
| R015 | multi-row DMA | batching DMA; out of scope |
| R016 | shape-aware rows/task alone | **parent baseline only**; scheduling without row-group. V001 adds R012 on top |
| R017 | FP32 full middle | precision |
| R018 | CAST_RINT output | store mode |
| R019 | invRms divide order | arithmetic order |
| R020 | sqrt normalization form | arithmetic form |
| R021 | CANN9 build chain | infra |
| R022 | codegen | codegen |
| R023 | engineering convention | process |
| R024 | CPU validation matrix | QA |
| R025 | submission integrity | infra |
| R026 | msprof measurement | tooling |
| R027 | GPU migration notes | research |
| R028 | scalar sync reduction | reduction/sync |
| R029 | official five-mode taxonomy | dispatch taxonomy naming; different mechanism |

Scheduler-only (R016) and row-group-only (R012) were tested as **separate** historical routes with separate scores (R016 COMPILEFIX 17.14; PURE-R012 21.37). Neither historical result is the combination under one Direct Parent.

## Vs MIX / H001 / H003

- MIX-A V003 combines R31 multimode + A001 mechanisms (online 44.69); not R016×R012.
- H001 correctness-priority mix lists many ideas including R012 and R016 among R001/R003/R006/… but has **no valid official score** and is a correctness stack, not a scheduling+row-group single-hypothesis performance revision.
- H003 (R001+R007+R009+R010+R012+R016 tail-safe combo) is recorded **未开始** (never started) in 版本实验记录.md.

## Vs first six (R31A/R31B/MIX-A/WIDE-X-FRESH4/MODE-X-R015C/EXT-ASCEND-X)

- R31A/R31B: multimode exploit champions; not this pair.
- MIX-A: R31×A001 donor isolation.
- WIDE-X-FRESH4: wide-D blind; reduction/layout focus.
- MODE-X-R015C: multi-row strided DMA (R015), not R016+R012 ownership.
- EXT-ASCEND-X: external-derived rowFactor/ubFactor tiling (GUIDED_FRESH); different provenance and factor selection, not R016 shape bands + R012 32B group math.

## Vs other next6 sibling routes (by brief titles only; no cross-worktree source read)

- ALIGN-TAIL-X: tail/alignment specialist framing.
- ASYNC-TRIPLE-X: triple pipeline.
- BATCH-RESIDENT-X: batch + residency.
- REDUCE-INVSCALE-X: reduction/invRms.
- UB-LIVENESS-X: UB liveness/layout.

None is “R016 shape-aware rows/task + R012 32B row-group ownership”.

## Conclusion

Combination is new relative to every recorded scored or parked candidate in this worktree’s control/archive evidence. Separate halves were proven only as separate routes; H003 (nearest planned combo name) never ran.
