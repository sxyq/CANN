# Architecture Evidence Map

Labels: `PROVEN_WIN` | `PROVEN_LOSS` | `MIXED` | `INCONCLUSIVE` | `NOT_PROPERLY_TESTED`

Rule: compile failure != architecture disproven.

Sources: `phase4/control/results.tsv`, champion `result.json`, server3 repro, route READMEs / architecture-metadata (read-only).

| theme | label | evidence (summary) | confidence notes |
|---|---|---|---|
| multimode dispatch | PROVEN_WIN | R31A/R31B family holds 45.00–45.16; mode-specific buckets on T14/T15 | FACT: online Bests use multimode paths |
| row batching | PROVEN_WIN | R31 LP/row pipelines in V011; H001 multi-row tiles | FACT in champion + H001 path |
| parameter residency (gamma/bias) | PROVEN_WIN | A001 FastKernel param queue; R31 param cache in later revs | FACT: A001 V017 / MIX donor notes |
| cached-y / full-y / retained-u | MIXED | R31B V002 full-y win; R31A V016/V017 D-boundary regressions | FACT: V017 local win official loss |
| input reread reduction | PROVEN_WIN | R31A V016 change summary: D=32768 full-y avoids 2× input read; score 45.00 | FACT attribution V016 |
| wide-D full-y FP32 | MIXED | V016 helped; V017 extending 24576..32768 hurt official | FACT: 44.45 < 45.00 |
| D-slice cooperative | PROVEN_LOSS (online) | C001 TLE; multi-row D-slice family failed T14 historically | FACT online; not pure architecture proof |
| reduction redesign (ReduceSum / tree) | MIXED | H001 V007→V008 +score via ReduceSum; REDUCE-X three fails | FACT + route-specific fails |
| pipeline overlap (double-buffer) | MIXED | A001 V017 x/res 2-slot small win; R31 MTE depth flat on T14 | FACT A001; INCONCLUSIVE full triple |
| dtype specialization | MIXED | G001 low-precision negative on large-D; R31 BF16/FP32 split helps | FACT both directions |
| queue depth | MIXED | A001 V006 single-slot helped; deeper not always | FACT mixed |
| block/core topology (blockDim ↔ cores) | NOT_PROPERLY_TESTED | no clean online isolation in phase4 results | UNKNOWN beyond principles doc |
| UB layout | MIXED | I001 DataCopyPad tail proven; WIDE layout exploratory | FACT pad; layout sweep thin |
| epilogue fusion | NOT_PROPERLY_TESTED | EPI-X correctness FAIL before score | compile pass only |
| launch topology / host dispatch | MIXED | MODE-X host probes pass; no online | INCONCLUSIVE online |
| `sqrtf` in aicore | PROVEN_LOSS (compile) | WIDE-X-FRESH4 COMPILE_FAIL | FACT compile; architecture not disproven |
| GET_TILING macros in kernel.asc | PROVEN_LOSS | historical CE (phase3 notes) | infra rule |
| TensorGroupInfo redefinition | PROVEN_LOSS | R31A V001 CE | FACT |
| INPUT identity (partial/wrong kernel) | PROVEN_LOSS | 6ab35deb / 6ab3a527 CE ROOT_CAUSE INPUT_DIFFERENT | FACT infrastructure |

## Notes

- Hidden testcase shape is UNKNOWN. Do not invent shapes from score deltas alone.
- `PROVEN_LOSS` here means the specific formulation failed, not every related idea forever.
