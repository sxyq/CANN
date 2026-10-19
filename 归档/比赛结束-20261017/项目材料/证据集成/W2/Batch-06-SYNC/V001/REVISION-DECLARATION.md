# REVISION DECLARATION — SYNC-TOPOLOGY-CHAMPION-X V001

ROUTE=SYNC-TOPOLOGY-CHAMPION-X
REVISION=V001
DIRECT_PARENT=R31B-V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE=45.16
OFFICIAL_ANCHOR=45.16
HYPOTHESIS=H2
SINGLE_HYPOTHESIS=In ProcessWideLowPrecision pass 2, issue the existing next-slot gamma/bias DMA before waiting for the current slot's MTE2_V completion, while keeping that wait before current-slot parameter use.
FOCUS_AXIS=Ordering of the existing MTE2_V parameter-ready wait relative to next-slot MTE2 issue.
FOCUS_VALUE=The existing WaitFlag<MTE2_V>(prd) in ProcessWideLowPrecision pass 2.
ONE_FACTOR_DIFF=Move only WaitFlag<MTE2_V>(prd) from before the next-slot block to after its two gamma/bias Load calls and SetFlag<MTE2_V>, and before any V read of the current gamma/bias tile.
PROPOSED_ONE_FACTOR_DIFF=Same as ONE_FACTOR_DIFF; retain the existing V_MTE2 slot-release waits before slot reuse.
TARGET_SHAPE=rows=2,D=12288,blockCount=1; widePath_ is selected because D>8192, tileWidth=4096, tileCount=3, wideFullYRows_=3, and batchRows=2.
TARGET_DTYPE=FP16
EXPECTED_EFFECT=The next slot's existing parameter DMA commands can issue while the current slot's readiness wait is pending, potentially hiding MTE2 idle time. No numeric gain is assumed.
FAILURE_MODE=No measurable gain if MTE2 is already continuously occupied, the wait does not delay DMA issue, or the new overlap window is too short. An incorrect event wait or premature slot reuse could expose unfinished parameters; the declaration keeps current-slot consumption behind its existing MTE2_V wait and keeps V_MTE2 release waits before overwrite.
PRECISION_RISK=Arithmetic, casts, parameter values, and operation order are unchanged, so bitwise-equal output is expected. A synchronization-order defect could still expose incomplete gamma/bias data and fail correctness.
UB_DMA_SYNC_IMPACT=No UB allocation or buffer-depth change. Load count, DMA bytes, event IDs, event count, and arithmetic remain unchanged. One existing MTE2_V wait moves later within the tile iteration; no wait is removed.
CONTEXT_CLASS=OFFICIAL_CHAMPION_DERIVATIVE
WHY_NOT_DUPLICATE=R013/FULL-R013 establish general double-buffer pipeline precedent, but this revision changes no pipeline depth. R31B V017 defers an MTE3_V output-store wait in the low-precision path; it does not move the current gamma/bias MTE2_V readiness wait. ASYNC-OVERLAP-CHAMPION-X V001 moves the first parameter prefetch above the invRms tail; H2 leaves that prologue unchanged and only reorders later-tile issue against the current-tile wait. MIX-A V007 removes a V_MTE2 release sync in a narrow single-row path. R31B V019 removes SyncVToMTE2 full syncs. Those are different event directions or wait sites.
CROSS_ROUTE_DUPLICATE_AUDIT=R001-R029/FULL: R013 is broad double-buffer precedent; R028 is scalar GetValue/reduction synchronization; R015 changes multi-row DMA; R005 changes tile width; R016 changes row/core assignment, all outside this edit. R31A V021 delays MTE3_V output completion and lacks a qualified P/C result; R31A V028 removes a V barrier before SetFlag, not an MTE2_V wait. R31B V017 is the closest MTE3 overlap neighbor (Official 44.68); V018 removes pass-1 Muls/barrier work and V019 removes SyncVToMTE2 full syncs, neither tests this wait ordering. MIX-A V003 adds SyncVToMTE2 for reuse safety; V007 removes a V_MTE2 release wait and has no valid P/C signal. Wave-1 ASYNC-OVERLAP V001 prefetches the first parameter tile and scored 44.17 Official; it does not test later-tile MTE2_V wait ordering. Wave-2 STORE changes Store issue/writeback, EPI changes arithmetic grouping/order or work placement, SELECTIVE-FASTPATH evaluates the V017 donor, and SMALLMID changes small/mid dataflow. Their mechanisms remain unchanged here; the full per-lane comparison is in the committed route handoff.
MAIN_SELECTION_CONFIRMATION=Planning selected H2 per C2C CONTROL. Main-1 receipt and review are recorded in 研究/主代理/MAIN-1-W2/campaign-status.md at commit 2a27be0b.
EVIDENCE_PATHS=线上结果/R31B/V011/{submission.asc,submission.sha256,source-meta.json,result.json}; 研究/SYNC-TOPOLOGY-CHAMPION-X/TRACK-B-HANDOFF.md; 研究/主代理/MAIN-1-W2/campaign-status.md at 2a27be0b; 技术路线/全版本记录.tsv; 线上结果/R31B/V017/diff.patch and result.json; 线上结果/R31A/V028/diff.patch; 研究/R31B/handoff-v018.md at 7adf582c; 研究/R31B/handoff-v019.md at ebee3ded; 本地实验/MIX-A/V007/{diff.patch,handoff.md,local-result.json}; 线上结果/MIX-A/V003/diff.patch; 线上结果/ASYNC-OVERLAP-CHAMPION-X/V001/{diff.patch,result.json}; committed Wave-2 handoffs listed in the route handoff.
EXPECTED_LOCAL_PROBES=First, exact-source correctness and same-binary qualification for rows=2,D=12288,FP16,blockCount=1. D=8192 FP16 is the unmodified-path control. Only after a stable FP16 direction may rows=2,D=12288,BF16 be considered as a confirmation. Then use same-device interleaved parent/candidate device-event pairs per the project protocol. No build, NPU run, or timing is authorized by this declaration; Main must assign an independent device/job first.
SINGLE_CHANGE_AUDIT=PLANNED; only the relative position of one existing MTE2_V wait changes.
ONLINE_ACTION=NONE; Route Agent does not submit Online.
