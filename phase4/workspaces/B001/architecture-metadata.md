# B001 Architecture Metadata

## Identity

- Candidate: `B001`
- Route: True 2D Panel DMA and Broadcast on DAV_2201
- Target: Ascend 910B3 / `dav-2201`
- Source: `phase4/workspaces/B001/kernel.txt`
- Local compile entry: `phase4/workspaces/B001/main.asc`

## Kernel entry and dispatch

`run_kernel` derives `R = product(shape[:-1])`, `D = shape[-1]`, and dispatches one
of six typed entries. FP16, BF16, and FP32 each have a hot entry and a private
fallback entry. The hot entry is selected when the row width is 32-byte aligned;
otherwise the fallback uses the same complete dtype and shape domain with a
single-row panel.

## 2D UB layout

The hot path uses `panelRows=8` and `chunkCols=min(D, 512)` elements. Every
panel is stored row-major in UB with a fixed `rowAlignCols`, rounded to a
32-byte row start. The active layout is:

| Region | Shape | Role |
| --- | --- | --- |
| `xQueue` | `[panelRows, rowAlignCols]` | x panel from GM |
| `residualQueue` | `[panelRows, rowAlignCols]` | residual panel from GM |
| `gammaQueue` | `[1, rowAlignCols]` | one gamma chunk reused by all rows |
| `biasQueue` | `[1, rowAlignCols]` | one bias chunk reused by all rows |
| float work buffers | `[panelRows, rowAlignCols]` plus `[1, rowAlignCols]` | FP32 conversion and panel arithmetic |
| `outputQueue` | `[panelRows, rowAlignCols]` | converted output panel |
| reduction scratch | 8192 bytes | `ReduceSum<float>` temporary storage |

For FP16/BF16, the three panel input/output queues occupy 24 KiB at the
maximum hot-path chunk and the three FP32 panel buffers occupy 48 KiB. The
gamma/bias buffers and reduction scratch are included in the allocation above;
the complete maximum allocation is below 184 KiB, including queue depth two.
The fallback uses one row and a 256-element chunk, with queue depth one.

## Panel DMA

`LoadPanel` issues one `DataCopyPad` for x and one for residual with
`blockCount=rowsThis`. The source stride skips the remaining columns of each GM
row, while the destination stride advances by the aligned UB row. Gamma and
bias each use one vector copy per column chunk. `DataCopyPad` supplies zero
padding for D tails; output uses one multi-row panel copy with the inverse
strides.

## Broadcast reuse

Gamma and bias are loaded once per column chunk into `[1, rowAlignCols]` local
vectors. Every row in the panel applies the same local vectors with `Mul` and
`Add`; no row reload of gamma or bias occurs. The broadcast is therefore
panel-scoped and its source vectors remain resident while all panel rows are
computed.

## Multi-row reduction schedule

Each panel runs two column sweeps. The first sweep loads and computes all rows
in a panel and performs one `ReduceSum<float>` per row on the squared values,
accumulating the scalar partials across column chunks. The second sweep reloads
the same 2D panels, computes the row RMS inverse, applies gamma and bias, and
writes the whole panel. Tail rows use `rowsThis < panelRows` while preserving
the same panel layout. The fallback has the same two-sweep flow with
`panelRows=1` for complete non-aligned coverage.

## Core partition and workspace accounting

Host tiling computes `totalPanels=ceil(R/panelRows)`, launches at most the
available Vector Core count, and assigns contiguous panel ranges through
`panelsPerBlock`. No core outside the assigned range enters a barrier or waits.
The source does not use cross-core synchronization or a shared reduction
workspace; each row's reduction is complete within its owning panel core.

## Domain coverage

- Dtypes: FP16, BF16, FP32.
- Rank: 2D, 3D, and 4D are flattened into R rows.
- D: 64 through 32768, including non-32-byte-aligned widths.
- Rows: full panels and tail panels.
- Values: NaN and Inf propagate through IEEE arithmetic; epsilon is passed per
  invocation and is used in the row RMS denominator.
- Determinism: fixed row order, fixed chunk order, and no atomics.
