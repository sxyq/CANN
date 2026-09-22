# WIDE-X V001 Handoff

ROUTE: WIDE-X (non-D-slice wide-D, hierarchical UB reduction)
REVISION: V001
HYPOTHESIS: T14-class wide-D is reduce/re-read bound; keep full-row u in UB and two-level UB tree sum(u*u) (leaf ReduceSum + running Add) beats D-slice + SyncAll.
CHANGED: Fresh workspace WIDE-X; new submission.asc hierarchical UB reduction kernel + full dtype/rank run_kernel dispatch; no R31 D-slice.
COMPILE: device=OK (wide_x_device), submission=OK (wide_x_submission), full_link=OK (wide_x_full_link) on cann-server3 / dav-2201 / Ascend910B3.
SOURCE: /Users/sunyiyang/Desktop/Project/cann/phase4/workspaces/WIDE-X/submission.asc
COMPILE_LOG: /Users/sunyiyang/Desktop/Project/cann/phase4/workspaces/WIDE-X/logs/{configure_v001,compile_device_v001,compile_submission_v001,compile_fulllink_v001}.log
CLEANUP: server build/ kept under phase4-workspaces/WIDE-X/build; no source deleted.
ONLINE_READY: true (device+submission+full link + local smoke rank3 fp16 D=2048 max_abs=0.0078 bad=0)
EXPECTED_AFFECTED_CASES: 15/15 potential; focus wide D 4096..32768, few-row T14-class and large-R wide-D.
