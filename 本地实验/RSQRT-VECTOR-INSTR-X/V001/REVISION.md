# RSQRT-VECTOR-INSTR-X V001 declaration

- ROUTE: `RSQRT-VECTOR-INSTR-X`
- REVISION: `V001`
- DIRECT_PARENT: exact `R31B V011`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- PARENT_SCORE / OFFICIAL_ANCHOR: `45.16`
- CONTEXT_CLASS: `ROUTE_STUDIO_EXPLORE`
- SINGLE_HYPOTHESIS: in the two wide-low-precision invRms helpers only, replace the `Sqrt` + vector-to-scalar handoff + scalar reciprocal sequence with one `AscendC::Rsqrt` vector instruction writing the invRms value directly; retain the same reduction result, epsilon, output scaling, row ownership, and pipeline.
- WHY_NOT_DUPLICATE: this changes only the invRms instruction sequence in the wide-low-precision path. It does not use VECTOR-MATH's output denominator/broadcast transformation, change reduction topology, row ownership, cross-row scheduling, buffer use, or vector width.
- TECHNICAL_BOUNDARY: invRms math/instruction sequence only; no reduction topology, row ownership, or cross-row pipeline changes.
- RULE_SOURCE: required rules were read in this worktree at base commit `036aa82e9272ab743fc619aa12453a2cff8df8a0`.
- RULE_RECALL: one Route/Agent/context/branch/worktree; exact-parent provenance and Official anchor are separate; one variable per Revision; CHANGE → COMPILE → CORRECTNESS → LOCAL → RESULT → COMMIT; correctness gates Local; compare exact Parent and Candidate under matching conditions and retain raw evidence; keep Local separate from Official; retain failures; stage explicit Route-local paths only; no `git add .`, force push, reset, rebase, or clean; no shared-file edits or Online; after Local result classify briefly, commit, and start next edit within 120 seconds.
- DECLARATION_STATUS: `DECLARED_BEFORE_CANDIDATE_EDIT`
