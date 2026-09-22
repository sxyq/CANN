# REDUCE-X V002 HANDOFF

ROUTE: REDUCE-X
REVISION: V002
HYPOTHESIS: V001 100% WA + T03 RE was Brcb workspace overflow (Brcb writes 8x32B=256B per repeat into a 32B scratch), plus ReduceSum dst 8B misalignment on odd batch slots. Fixing workspace size + alignment restores Brcb zero-VS numerics; tiny shapes use GetValue golden to lock 15/15.
CHANGED: (1) Brcb dest workspace sized to 64 floats / 256B. (2) ReduceSum dst slots stride-2 for 8B alignment. (3) GetValue golden path for D<=256 or tiny batches (T01/T02/T03 class): ReduceSum + SyncVToS + GetValue + Muls. (4) Zero-VS Brcb+tiled-Mul batch path for larger hot rows. (5) Hierarchical two-pass kept for D>8192. (6) Dedicated 512B work + 8KiB reduce tmp.
COMPILE: device compile PASS; submission compile PASS; full link PASS (cann-server3, Ascend910B3, dav-2201, CANN 8.5.0.alpha002)
SOURCE: /Users/sunyiyang/Desktop/Project/cann/phase4/workspaces/REDUCE-X/submission.asc
COMPILE_LOG: /Users/sunyiyang/Desktop/Project/cann/phase4/workspaces/REDUCE-X/compile.log
CLEANUP: none required this round
ONLINE_READY: yes
EXPECTED_AFFECTED_CASES: all 15 (T01/T02/T03 via golden; remaining via fixed Brcb batch + hierarchical + fallback)
REDUCTION_DESIGN: multi-row ReduceSum into 8B-aligned sum[2i]; golden = GetValue+Muls; zero-VS = Brcb 256B work -> Rsqrt inv8 -> 8-wide tiled Mul; wide = hierarchical chunk Add + final ReduceSum
V_S_HANDOFF_COUNT_PER_ROW: 0 on zero-VS batch and wide; 1 on golden/fallback (tiny)

## Root cause (V001 -> V002)

| Bug | Effect | Fix |
|-----|--------|-----|
| Brcb scratch 32B but writes 256B | 100% WA, UB corruption, T03 RE | work buffer 64 floats |
| ReduceSum dst at odd float slots | misaligned reduce, wrong sum | sum[2*i] stride |
| Tiny-shape Brcb association untested | T01/T02 WA | GetValue golden for D<=256 |

## Preflight

- 5/5 template checks PASS
- SHA-256: 0419b9a47fba2f998ac1bc16bb969a245747b4cbb90c59968b064b6b45ea4fc8
- 行数 612 / 字节 20711

## Next experiment

After online 15/15: switch tiny shapes from golden to fixed Brcb and re-measure V/S win; overlap MTE with reduce (TQue depth 2).
