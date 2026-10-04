# TILECOUNT Axis Exhausted — Planning Fact Pack

```text
DATE_UTC=2026-10-04
ROUTE=TILECOUNT-STATIC-UNROLL-CHAMPION-X
TRACK=GATE-FACT-SUMMARY
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TILECOUNT_AXIS_EXHAUSTED=YES (scope below)
MAIN_SELECTED=NONE
REVISION=NONE
```

## Formal gate result

The bounded final gate dated 2026-10-03 records **TCSU-H1, H2 and H3 = FAIL**. All three require proof that the exact parent emits loop-control/index work which a source expansion can remove; source-level fixed trip count is not that proof.

| Hypothesis | Exact parent site and bounded potential | Formal FAIL reason |
|---|---|---|
| TCSU-H1 | `ProcessFp32FullRowOutputPipelined()`, parameter preload (`submission.asc:1129–1132`): two gamma and two bias loads at offsets 0/4096, once per block. | Exact-parent codegen is uninspectable; potential is setup-only and bounded. |
| TCSU-H2 | First pass (`:1164–1180`): N=2 per row; at `localRows=4`, eight tile bodies per block. | No evidence that the exact parent retains a backedge/index cost, hence no attributable instruction delta. |
| TCSU-H3 | Output pass (`:1203–1225`): N=2 per row; preserves two stores and existing event selection/order. | Loop and event-selection codegen cannot be inspected; no parent control cost is proven. |

The exact-source parent build did succeed (`/home/data4t2/lelinfeng/server_full_audit/logs/R31B__V011.build.log`, RC 0; object SHA-256 `97dbdc89fa7f125d345d9c3ecd919f5d62277986ff27987a576abaeedaf78855`). However, the retained device binary could not be disassembled: the extracted `hiipu` `.text` was reported `<not available>` by the bundled `llvm-objdump`, and the vendor compiler rejected `-S` / multi-output source emission. Thus `EXACT_PARENT_CODEGEN=UNVERIFIED` for all three.

**Interpretation:** these are gate failures, not performance, correctness, or timing results. They do not establish that the compiler already unrolled the loops; they establish that the required evidence of remaining parent work was unavailable. This task performed no build, correctness run, NPU work, profiling, or timing.

## Exhaustion boundary

`TILECOUNT_AXIS_EXHAUSTED=YES` is limited to the current admissible pool of three source-level, one-loop N=2 expansions in the exact R31B V011 FP32 path `rowWidth == kCacheElems && localRows > 1`, probed at FP32 `[128,8192]`, 32 cores / blocks, `localRows=4`. Under that boundary, there is no fourth distinct static-unroll hypothesis in the audited pool.

It does **not** assert that every tile-size, shape-tiling, dtype, dispatch, or code-generation optimization is exhausted; those change different axes and may have separate route ownership. It is not a `PARK`, `CLOSE`, `ABANDON`, `MERGE`, or `REPLACE` decision.

The three gate results close the present H1/H2/H3 eligibility checks. Do not retry or rename these same source-loop expansions, and do not create a Revision from this pack. Any future reconsideration would require Planning direction and genuinely new exact-parent device-codegen evidence; it is not authorized by this fact summary.

## Separate facts available to Planning

These are ownership/context facts, **not** replacement hypotheses or route selections:

- **Tile geometry / UB:** R005 changes to an 8192-element work tile and a different UB plan; that is explicitly outside TCSU's frozen 4096/8192 geometry and allocation. See `TRACK-B-POOL-EXHAUSTION-AUDIT-20261001.md` §4.
- **Runtime mode / shape policy:** R029 covers mode/shape dispatch and wide-row behavior; SHAPE-TILING covers shape-conditioned tile/UB tradeoffs and is a separate Main-1 research ownership. Neither is evidence for residual codegen in this exact N=2 parent loop.
- **Generated schedule:** R022's source generator is not an inspection artifact for the compiled R31B V011 device instructions and does not close this gate.
- **Evidence bottleneck:** the parent host object/build identity is retained, but a usable exact-parent AICore disassembly/codegen view is not. The gate blocker is observability, not a demonstrated kernel bottleneck or measured gain.

Planning may use these boundaries when deciding whether to assign a distinct, correctly owned line of inquiry. This pack makes no such selection.

## Evidence index

- `TILECOUNT-FINAL-GATE.md` — formal H1/H2/H3 FAIL and exact-parent binary/tool limitation.
- `TRACK-B-CODEGEN-DUPLICATE-AUDIT-20261001.md` — fixed-N source proof, codegen gate, and cross-route boundaries.
- `TRACK-B-POOL-EXHAUSTION-AUDIT-20261001.md` — three-candidate pool and historical R005/R029/R022 distinctions.
- `TILECOUNT-PLANNING-HANDOFF.md` / `TRACK-B-HANDOFF.md` — prior research scope and unchanged `MAIN_SELECTED=NONE` status.
