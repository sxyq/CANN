# TILECOUNT Final Candidate Gate

```text
DATE_UTC=2026-10-03
ROUTE=TILECOUNT-STATIC-UNROLL-CHAMPION-X
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TARGET=FP32 [128,8192], availableCoreNum=32, blockCount=32, localRows=4
MAIN_SELECTED=NONE
REVISION=NONE
IMPLEMENTATION=NONE
```

This is the single bounded final gate. No Parent source was changed, no
Candidate was implemented, and no NPU correctness/performance/profiling run
was made. `PASS` requires evidence that the exact Parent retains relevant
loop-control/index work; source-level trip count alone is insufficient.

## Exact-parent codegen evidence

- The audited Parent source hash matches the retained source and the existing
  server audit source: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Existing exact-source build: `/home/data4t2/lelinfeng/server_full_audit/logs/R31B__V011.build.log`, return code 0; source `/home/data4t2/lelinfeng/server_full_audit/src/R31B__V011.asc`.
- Toolchain/options: CANN 8.5.0.alpha002, `bisheng`, `--npu-arch=dav-2201`, `--npu-soc=Ascend910B3`, `-O3`, C++17. Existing object SHA-256: `97dbdc89fa7f125d345d9c3ecd919f5d62277986ff27987a576abaeedaf78855`.
- The object contains `.aicore_binary`; extracting it gives ELF machine `hiipu` / `0x1029`. The bundled `llvm-objdump` reports no embedded AICore binaries for the host object and prints `<not available>` for the extracted device `.text`. The vendor compiler rejects `-S` (`unsupported option '-S'`); direct `-S -o` also cannot represent its multiple outputs. Therefore the available exact-parent binary proves the compile identity, but does **not** reveal whether these source loops have backedges or equivalent dynamic indexing.
- Thus `EXACT_PARENT_CODEGEN=UNVERIFIED` for H1/H2/H3. This means the mandatory gate is not met; it does not mean the compiler definitely unrolled the loops.

## Candidate checks

| ID | Exact source site / actual loop | Fixed trip count and hot-path role | One-factor / target / expected code effect | Duplicate audit | Implementable | Result / reject reason |
|---|---|---|---|---|---|---|
| TCSU-H1 | `ProcessFp32FullRowOutputPipelined()`, parameter preloads at `submission.asc:1129–1132` | `8192 / 4096 = 2`; once per block, before the row loop (setup, not a per-row hot loop) | Expand only the two gamma/bias loads at offsets 0 and 4096; same FP32 `[128,8192]` path. If a backedge/index sequence exists, remove one two-trip setup backedge and dynamic offset work per block. | Not ADDR address hoisting or parameter residency; four loads/order/destinations stay fixed. | NO (gate not passed) | FAIL — exact-parent codegen is uninspectable from retained output; setup-only potential is also bounded. |
| TCSU-H2 | Same function, first-pass tile loop at `submission.asc:1164–1180`, nested in the row loop | Two source trips per row; for the specified launch, four rows/core, hence eight tile bodies per block. It is the most repeated of the three candidates. | Expand only tile 0 and tile 1 at offsets 0 and 4096; keep first-tile prefetch condition, both `ReduceSum`s, both sync pairs, load opportunities and order unchanged. Potentially remove per-row tile-loop control/index work. | Not REDUCE-HIER if reduction calls/order/syncs are identical; any reduction or barrier change is duplicate/out of scope. | NO (gate not passed) | FAIL — exact-parent backedge/index work is unverified, so there is no demonstrated instruction delta to justify this source change. |
| TCSU-H3 | Same function, output tile loop at `submission.asc:1203–1225`, nested in the row loop | Two source trips per row; four rows/core for this launch, eight output bodies per block. | Expand only the two output tiles at offsets 0 and 4096; preserve `useFirst`, event IDs, waits, stores/drain order, and `Muls→Mul→Add`. Potentially remove per-row loop control and constant event selection. | Not BRANCH guard hoisting if generic guards/valid lengths are untouched; changing event/store policy is duplicate/out of scope. | NO (gate not passed) | FAIL — exact-parent loop and event-selection codegen cannot be inspected; no relevant Parent control cost has been proven. |

All three source loops remain present in exact V011 source, with compile-time
N=2 under the stated FP32 path. Only that source-level fact is proven. The
compiler's treatment is unknown; no candidate is selected from unverified
potential.

```text
TCSU-H1=FAIL
TCSU-H2=FAIL
TCSU-H3=FAIL
MAIN_SELECTED=NONE
TILECOUNT_LOCAL_SCORE=NONE
NEXT=Planning review; select no V001 from this gate without new exact-parent codegen evidence.
```
