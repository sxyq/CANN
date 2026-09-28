# WHY_NOT_DUPLICATE — UB-LIVENESS-X V001

## vs existing 29 routes (idea-pool-29-routes.md)

The 29-route pool never makes **phase-role reuse of one physical UB pool** the primary
mechanism. Nearby IDs only touch one side of the budget:

| ID | What it does | Why UB-LIVENESS-X differs |
|----|--------------|---------------------------|
| R005 large tile | grows tile bytes | does not free peak live-set; often fights param/tmp residency |
| R013 double-buffer | adds depth-2 queues | adds slots **on top of** permanent x/res/u/out regions (more peak UB) |
| R014 param residency | keeps gamma/bias | typically requires permanently separate buffers; no ingest→emit role swap |
| R015 multi-row DMA | changes DMA shape | scheduling/DMA organization, not buffer lifetime |
| R002 resident-y | retains output/u | keeps a **permanent** y role; no phase reassignment of that memory |
| R006/R007/R011 reduce | reduction form | compute topology, not UB liveness |
| R029 five-mode | dispatch taxonomy | mode selection, not physical aliasing |

V001’s single change: one physical pool is **INGEST (x/res depth-2 tiles) → FUSE (u
resident or streaming) → EMIT (out staging aliases former x/res bytes)**, with PARAM
anchored. Peak simultaneous live bytes drop so the same 184 KiB buys larger tile
and/or full param residency and/or pipeline slots.

## vs R030 UB reuse (FULL-R030-WIDE-PARAM-REUSE)

R030 evidence (`exp__full-r030-wide-param-reuse-v001/.../结果.md`) only **shrinks**
the wide-param working set (tile 6912→4096, retained-y as half) to dodge
`ub address out of bounds`. That is footprint reduction of dedicated roles.

UB-LIVENESS-X does not shrink dedicated roles; it **eliminates simultaneous roles**
by reassigning the same bytes across phases. Expected outcome is not merely “fits
UB” but **more** param residency + **larger** tile + pipeline slots at once.

## vs MIX-A

MIX-A isolates donor mechanisms (sync removal, D-split, FastKernel gates) on an
existing multi-path candidate. V001 is GUIDED_FRESH from task ABI: fresh kernel,
one architectural lever (UB liveness/alias), no MIX donor stack.

## vs first six (R31A/R31B/MIX-A/WIDE-X-FRESH4/MODE-X-R015C/EXT-ASCEND-X)

| Lane | Focus | Overlap? |
|------|-------|----------|
| R31A/B | multimode exploit / wide seed | mode thresholds & wide tile, not pool liveness |
| MIX-A | mechanism isolation on hybrid | different parent lineage |
| WIDE-X-FRESH4 | wide-D blind | tile/D strategy |
| MODE-X-R015C | mid multi-row DMA | DMA shape |
| EXT-ASCEND-X | rowFactor/ubFactor tiling factors | tiling knobs, not phase-role alias |

None of the six makes cross-phase buffer identity the hypothesis under test.

## vs other next6 lanes (names only; no foreign source read)

| next6 route | Stated lane | Why distinct |
|-------------|-------------|--------------|
| ASYNC-TRIPLE-X | async / triple overlap | scheduling & pipe stages, not UB byte ownership |
| BATCH-RESIDENT-X | batch residency | keeps data resident (new live region), opposite of recycling |
| REDUCE-INVSCALE-X | inv-scale reduction | reduction math path |
| SCHED-ROWGROUP-X | row-group scheduling | core/row assignment |
| ALIGN-TAIL-X | alignment / tail | pad & tail correctness |
| UB-LAYOUT-X (queue) | static layout exploration | layout partitioning without phase-role identity as the OFAT lever |

V001 does not implement triple pipeline, batch-resident retention as the lever,
inv-scale reduce, row-group schedulers, or tail-special paths.

## OFAT

One conceptual change: phase-role UB liveness/aliasing. Math (u=x+res; RMS;
gamma/bias), dtype path, fallback duty, and row-parallel core mapping are baseline
correctness scaffolding, not concurrent performance hypotheses.
