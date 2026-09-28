# MODE-X V001 Handoff

ROUTE: MODE-X (R029 official tiling five-mode dispatch, own kernel ABI)
REVISION: V001
HYPOTHESIS: Runtime selection of SINGLE_N / MERGE_N / MULTI_N / SPLIT_D / NORMAL from (D, R, UB) beats a single fixed schedule across 64..32768 and R=1..large; SPLIT_D is intra-core D-tile running reduce (not cross-core SyncAll D-slice), NORMAL is the two-pass fallback.
CHANGED: Fresh workspace MODE-X; new submission.asc five-mode dispatch kernel + FP32 intermediate full dtype/rank run_kernel; UB-budget-derived tile (SelectTile) and batch (SelectMergeK); no R31 compute copy; no official tiling ABI.
COMPILE: device=OK (mode_x_device), submission=OK (mode_x_submission), full_link=OK (mode_x_full_link) on cann-server3 / dav-2201 / Ascend910B3 / CANN 8.5.0.alpha002.
SOURCE: /Users/sunyiyang/Desktop/Project/cann/phase4/workspaces/MODE-X/submission.asc
COMPILE_LOG: /Users/sunyiyang/Desktop/Project/cann/phase4/workspaces/MODE-X/logs/{configure_V001_20260922_182606,compile_device_V001_20260922_182606,compile_submission_V001_20260922_182606,compile_fulllink_V001_20260922_182606,build_summary_V001_20260922_182606}.log
CLEANUP: server build kept at /home/data4t2/lelinfeng/phase4-workspaces/MODE-X/build; no source deleted; local logs synced.
ONLINE_READY: true (device+submission+full link; ABI checks cmath-first, vector entry, run_kernel, rightPadding-bytes, ReduceSum 8KiB, launch nullptr smdesc, no TensorInfo redefine)
EXPECTED_AFFECTED_CASES: 15/15 potential; focus T14/T15 wide D (SPLIT_D), mid-D multi-row (MERGE_N), large-R (MULTI_N), R=1 full-u (SINGLE_N).

## MODE_TABLE

| Mode | Trigger (UB / D / R) | Core mapping | Compute |
|------|----------------------|--------------|---------|
| SINGLE_N | full-u fits, R==1 | 1 core | full FP32 u in UB; one x/r read; normalize from resident u |
| MERGE_N | full-u fits, R>1, K>=2 rows batch fits, D<=8192 | ceil(R/K) batches, sliced over cores | multi-row contiguous DataCopyPad of K rows (x,r); per-row RMS; shared u workspace |
| MULTI_N | full-u fits, R>1, not MERGE_N | min(R, cores) row-parallel | each core owns a row slice; per-row full-u (SINGLE_N inner) |
| SPLIT_D | full-u does not fit under 75% UB budget, tile fits | min(R, cores) row-parallel | D tiles + running sum(u*u); two-phase inside core (no SyncAll); tile = SelectTile(UB) |
| NORMAL | tile degenerate / last resort | min(R, cores) row-parallel | two-pass fallback, chunk-sized tiles, always-correct |

UB budget: 180 KiB of 184 KiB TOTAL_VEC_LOCAL_SIZE; ReduceSum tmp 8 KiB 8B-aligned dest; DataCopyPad rightPadding in BYTES; launch `<<<blockDim, nullptr, stream>>>`.
