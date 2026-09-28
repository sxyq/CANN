# 29-Route Idea Pool (read-only synthesis)

Source (route notes / results / principles only — no implementation restored):
- `归档/phase3-before-reset-20260920/管理/路线状态/R001.json`–`R029.json`
- `归档/phase3-before-reset-20260920/提交/单方案/R00N-*/README.md`
- `归档/phase3-before-reset-20260920/提交/版本实验记录.md` (summary level)
- Phase4 evidence: `phase4/control/results.tsv`, champion/fresh online results

Columns: idea · previous evidence · covered_by_current_phase4 · still_unexplored · reason_to_retry

| ID | idea | previous evidence | covered_by_current_phase4 | still_unexplored | reason_to_retry |
|----|------|-------------------|---------------------------|------------------|-----------------|
| R001 | 两遍扫描 two-pass sum then apply | standard baseline family | A001/H001/I001 two-pass | — | covered |
| R002 | 单遍 y 驻留 resident-y | G001 family; MIX-R014-R002 | G001 park; A001 full-u; R31 full-y wide | vectorized one-read without scalar GetValue | G002-class |
| R003 | 纯 Ascend C 直调 | compile path | all phase4 kernels | — | covered |
| R004 | 低精度中间计算 | historical BLOCKED | G001 proved intermediate native cast breaks large-D | dtype-split accumulators only in safe ranges | only if gated |
| R005 | 大 Tile | R005/FULL large-tile | I001 large-tile 2048 | tile autotune per D bucket | MID-X/WIDE-X |
| R006 | 分块归约 chunked reduce | historical | H001 wide chunks | hierarchical UB tree | REDUCE-X |
| R007 | ReduceSum | historical + H001 V007/V008 jumps | H001 hot+wide; I001 tmp | multi-row simultaneous ReduceSum | REDUCE-X |
| R008 | Tile 跨核 tile-across-cores | PLANNED only | D-slice variants FAILED for T14 | true tile pipelining across cores without D-slice name | WIDE-X if different mechanism |
| R009 | DataCopyPad 尾块 | historical | I001 V004 proven | — | covered |
| R010 | 手工尾块 manual tail | PLANNED | partial via pad | software mask epilogue for non-32B D | MID-X optional |
| R011 | 手工向量归约 manual vector reduce | PLANNED | pairwise G001 V008 no WA fix | full vector tree with 0–1 GetValue | REDUCE-X primary |
| R012 | 32B 行块对齐 | historical | widespread | — | covered |
| R013 | 双缓冲 double-buffer | R013/FULL pipeline; A001 V017 green | A001 x/res DB; R31 MTE2/MTE3 depth | full MTE2/V/MTE3 triple overlap wide | WIDE-X/MID-X |
| R014 | GammaBias 驻留 | historical; MIX-R014 | A001/H001/R31 param cache | stripe-resident params for D>UB | WIDE-X |
| R015 | 多行合并 DMA multi-row DMA | historical | H001 multi-row tile | true multi-row DataCopy with stride | MID-X primary |
| R016 | 按行分配 AI Core | historical | default row-parallel | row+col hybrid without D-slice naming (MIX-A D-split) | MIX-A |
| R017 | FP32 全中间 | historical | G001/A001/R31 | — | covered |
| R018 | CAST_RINT 输出 | historical; G001 V004 neutral/nick | tested | only if positive mismatch evidence | low priority |
| R019 | Divs 先除 / invRms | historical; G001 V003 broke large-D | tested | invRms without native-u quantize | careful REDUCE-X |
| R020 | Sqrt 归一化 | historical | all | rsqrt primitive alternative | REDUCE-X |
| R021 | CANN9 构建链路 | historical | COMPILE_CONTRACT | — | infra |
| R022 | 自动代码生成 | PLANNED | not used | dtype×D-bucket codegen | late |
| R023 | 工程规范 | historical | — | — | process |
| R024 | CPU 验证矩阵 | historical | limited | CPU golden per dtype/D before online | Main/agent QA |
| R025 | 提交通道完整性 | historical | SUBMISSION_CONTRACT | — | infra |
| R026 | 性能测量 msprof | historical | online times only | local msprof Kernel task time to map modes | Main analysis |
| R027 | GPU 迁移参考 | research only | — | epilogue fusion / D-bucketing / warp-free ideas | REDUCE-X/MID-X notes |
| R028 | 标量同步削减 | PLANNED; H001/A001 GetValue cuts helped | partial (single GetValue) | multi-row batched RMS scalars; SPR if supported | REDUCE-X primary |
| R029 | 官方 Tiling 五模式 | PLANNED | R31 multimode loosely related | SPLIT_D / SINGLE_N / MERGE_N / MULTI_N / NORMAL as **dispatch taxonomy** without copying official tiling ABI | MIX-A / R31B |

## ≥5 still-unexplored ideas for replaceable slots

1. **R011 + R028 — vector reduction tree with batched multi-row RMS scalars (0–1 GetValue per row-group)**  
   Covered only partially (ReduceSum + one GetValue). Not covered: multi-row simultaneous reduce + batched scalar handoff + reduce/DMA overlap as one design. Owner: REDUCE-X.

2. **R015 — true multi-row combined DMA (stride DataCopy) for mid D**  
   H001 multi-row tiles exist but not a dedicated stride multi-row DMA redesign. Owner: MID-X.

3. **R013 wide triple overlap — MTE2 / V / MTE3 full pipeline on wide rows (not just 2-deep one side)**  
   R31 tried MTE2 depth and MTE3 depth separately; both flat on T14. Full triple-buffer schedule untested. Owner: WIDE-X.

4. **R029 five-mode dispatch taxonomy (SPLIT_D / SINGLE_N / MERGE_N / MULTI_N / NORMAL) inside kernel**  
   R31 multimode is related but not this taxonomy; no phase4 route implements official five-mode selection logic. Owner: MIX-A or R31B.

5. **R010 + R027 — manual tail/epilogue fusion (normalize+gamma+bias fused after one u load) with software mask**  
   Epilogue fusion mentioned in GPU notes; not implemented as a fused vector epilogue on Ascend. Owner: REDUCE-X / MID-X.

6. **R008 without D-slice branding — tile-across-cores producer/consumer (one core MTE2, neighbor V) if ISA allows**  
   Distinct from D-slice reduction. Unexplored. Owner: WIDE-X if legal on DAV_2201.

7. **R004 safe low-precision accumulator only for mean(u*u) in FP16 when D small and |u| bounded**  
   Historical BLOCKED + G001 negative evidence for unguarded casts — retry only with strict range gates. Low priority.

## Replacement guidance
- If MIX-A / WIDE-X / MID-X / REDUCE-X stall 2–3 architecture-level experiments with no score gain, no local case win, and no new conclusion: park and pull from items 1–6 above.
