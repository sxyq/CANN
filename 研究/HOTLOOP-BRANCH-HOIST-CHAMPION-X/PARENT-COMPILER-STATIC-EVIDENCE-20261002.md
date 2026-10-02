# HOTLOOP-BRANCH-HOIST-CHAMPION-X — parent compiler/static evidence

## Identity

- DATE: 2026-10-02
- DIRECT_PARENT: `R31B V011`
- SOURCE: `线上结果/R31B/V011/submission.asc`
- SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SOURCE_LINES: `3554`
- SOURCE_EDITED: `NO`

## Parent-only compiler probe

The exact parent source was compiled without creating a Candidate or Revision,
using the installed CANN `8.5.0.alpha002` Ascend compiler for `Ascend910B3`,
`dav-c220`, `--npu-arch=dav-2201`, C++17, and `-O3` for the AICore side. An
existing ABI-only preinclude supplied the judge metadata declarations needed to
compile the retained host wrapper; it did not alter the parent source.

- Bisheng: clang `15.0.5` (`bisheng --version`)
- Parent compile return code: `0`
- Parent object SHA256: `965b6876e7ad1c0e906dc5259d9c2ddee89ab79ef1766be67f237a6ef6a2b3a7`
- Object evidence: `.aicore_binary` size `0xACD8`; `__aicore_rel_binary` size
  `0xB4D1`; all three host dtype launcher instantiations are present.

This is a compile/object identity fact only. It is not a correctness or timing
result.

## IR/disassembly limitation

- The mixed `.asc` driver rejects `-emit-llvm` with
  `bisheng: error: -emit-llvm cannot be used when linking`; no device LLVM IR
  was therefore retained.
- The extracted device section is recognized as `elf64-hiipu`, but the
  available `llvm-objdump` reports `<not available>` for its device
  instructions. No source-line-correlated AICore branch disassembly was
  available.
- The auxiliary host optimization record contained no `loop-unswitch` record,
  but it only describes host/plugin wrapper functions and is not evidence of
  AICore loop optimization.

Accordingly, all runtime branch outcomes in the planning pack are
`COMPILER_STATUS=UNVERIFIED`. `if constexpr` dtype selection is the only
compile-time elimination asserted: for each instantiated T, the other dtype
arms are absent by language semantics. No runtime `cacheRow/cacheParams`,
`u&1`, `tile&1`, or `u+1<units` condition is claimed compiler-eliminated.

## Static branch inventory

| ID | Parent source condition | Loop level | Parent-only conclusion |
|---|---|---|---|
| HBH-09 | `!cacheRow || !cacheParams` at 393 and dtype-selected 464/476/485 | Generic row → output-tile loop | Loop invariant in source; repeated per-tile decisions are present in source; device hoisting is `UNVERIFIED`. |
| HBH-10 | `useA=(u&1)==0`, current slot selects 3135–3139, lookahead `nextA=!useA` selects 3155–3160 | Wide batch → flattened pass-1 unit loop | Parity changes with runtime `u`; no static hoist; emitted lowering is `UNVERIFIED`. |
| HBH-11 | `useA=(tile&1)==0` and parameter slot selects 3273–3276; lookahead 3278–3297 | Wide batch → pass-2 parameter-tile loop | Parity changes with runtime `tile`; no static hoist; emitted lowering is `UNVERIFIED`. |
| HBH-12 | `u + 1 < units` at 3146 | Wide batch → flattened pass-1 unit loop | Runtime terminal guard is source-real; removal would require loop/schedule restructuring, so it is rejected as non-independent. Compiler elimination is not claimed. |

## Static invariants and tail facts

- Generic path: `tileCount=ceil(rowWidth/4096)` at 292–294 and every output
  tile uses `valid=TileLength(rowWidth-col)` at 370–371. HBH-09 must preserve
  the final-tile FP32 drain at 472–473 and all output byte counts.
- Wide path: `tileWidth=wideFullYTileElems_` and
  `tileCount=ceil(rowWidth/tileWidth)` at 3089–3093. Pass 1 has
  `units=batchRows*tileCount` at 3122; its lookahead body executes `units-1`
  times per batch. Pass 2 parameter selection executes `tileCount` times and
  its next-tile body `tileCount-1` times per batch, while stores repeat for
  each `batchRow`.
- The wide current and lookahead valid values at 3131–3154 and 3269–3294 are
  runtime min expressions. A partial final tile, row-boundary lookahead, and
  terminal no-lookahead behavior must remain byte/order identical.
- `ProcessWideLowPrecision()` is covered only for non-float wide inputs
  (`rowWidth>8192`); generic HBH-09 is covered only after the existing
  special returns. No source evidence authorizes a new D/dtype gate.

## Closure

`MAIN_SELECTED=NONE`; `REVISION_CREATED=NO`; `CANDIDATE_CREATED=NO`;
`FORMAL_TIMING=NO`; `ONLINE=NO`. This file records parent source/compiler
evidence and unavailable proof boundaries only. It contains no generated
Candidate source and no shared-ledger/Dashboard update.

