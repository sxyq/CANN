# HOTLOOP-BRANCH-HOIST-CHAMPION-X V002 Revision Declaration

## Control and lineage

- `ROUTE`: `HOTLOOP-BRANCH-HOIST-CHAMPION-X`
- `REVISION`: `V002`
- `TRACK`: `A` / authorized sibling experiment
- `MAIN_SELECTED`: `YES` (本轮控制授权)
- `HYPOTHESIS`: `HBH-09`
- `BRANCH_KIND`: `GENERIC_REUSE_FENCE_GUARD`
- `DIRECT_PARENT`: `R31B V011`
- `PARENT_SOURCE`: `线上结果/R31B/V011/submission.asc`
- `PARENT_SOURCE_SHA256`: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- `PARENT_OFFICIAL`: `45.16`
- `SIBLING_LINEAGE`: `R31B V011 -> V002`
- `V001_LINEAGE`: `V001` is `LOCAL_REJECTED` and restored; it is not this revision's parent
- `CONTEXT_CLASS`: `MAIN-2_WAVE-2_BRANCH_OFAT_GENERIC`

## Why this is authorized

The external control for this turn authorizes exactly one HBH-09 sibling after
HBH-10 V001 final closure and its explicit byte-for-byte restore. The historical
planning pack's `MAIN_SELECTED=NONE` is superseded only by that explicit turn
authorization; no other hypothesis is selected here. V002 is created directly
from the restored official-backed R31B V011 bytes, not from the rejected HBH-10
V001 chain.

## Exact one-factor change

Factor only the generic `Process()` reuse-fence decision
`!cacheRow || !cacheParams` from the output-tile loop into a row-scoped
fence-mode selection. The selected row loop must preserve the same fence count,
order, operands, `valid`, row/tile offsets, loads, arithmetic, stores, and the
FP32 final-tile `SyncMTE3ToV()` drain. The intended implementation may use two
runtime-counted output-loop bodies selected once per row; it must not remove or
move any synchronization.

The primary qualification domain is generic `D=6144` with FP32 and BF16,
covering one-row (`localRows=1`) and multi-row (`localRows>1`) modes. FP16 at
`D=6144` is the control instantiation. These probes remain on the existing
generic path and record the actual `localRows`/block geometry; no new dispatch
gate is introduced.

## Why not duplicate another mechanism

- No HBH-10 or HBH-11 wide pass slot selection, parity state, tile change, or
  synchronization change is included.
- No HBH-12 terminal-guard removal, loop peeling, fixed-trip form, address
  cursor, DMA change, UB allocation, reduction change, arithmetic change, or
  output-store change is included.
- No host wrapper, dtype dispatch, wide threshold, specialized-path selection,
  shape bucket, or new donor is included.
- Unlike coefficient-mode factoring, this revision changes only the repeated
  generic reuse-fence decision; parameter operands and their load/reuse choices
  remain byte/order equivalent.

`SINGLE_CHANGE_AUDIT`: `PASS_INTENDED_SINGLE_HBH-09_REUSE_FENCE_FACTORING`

## Risks and gates

- The compiler may retain the repeated branch (`CODEGEN_NOOP`) or duplicate the
  loop with code-size/register pressure that erases any benefit.
- The implementation must preserve the exact mode-specific MTE2/V/MTE3 fence
  protocol, including the FP32 deferred final-tile MTE3 drain.
- Generic FP32/BF16/FP16 instantiations may have different emitted code and UB
  pressure; source identity, executable identity, and targeted correctness are
  independent gates.
- No same-binary or formal local timing is allowed unless build/link,
  correctness, exact identity, legal device lease, and the prescribed
  45-warmup device-event protocol all pass.

`ONLINE`: `NOT_PERMITTED` (Route Agent does not submit Online)

