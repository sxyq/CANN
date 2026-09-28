# WIDE-X V002 Handoff

ROUTE: WIDE-X (non-D-slice wide-D, hierarchical UB reduction)
REVISION: V002
HYPOTHESIS: T02 RE was tiny-path / TQue / count-overload / event-id race. Correctness first (chunked leaf ReduceSum + FetchEventID + full TQue free), hierarchical UB tree retained as the sumsq architecture (64-wide leaves → running Add).
CHANGED: submission.asc V002 RE-hardening — (1) explicit int32_t counts on all Level-2 ops, kVecChunk=64; (2) TQue EnQue/DeQue + FreeTensor on every slot (fixed FP32 yOut leak); (3) ReduceSum dest on dedicated 8B-aligned leafBuf + separate 8KiB tmp; (4) FetchEventID instead of shared event 0; (5) FP32 resident epilogue single full-row store; (6) inv_rms = 1/Sqrt + Newton polish; (7) tiny D=64 safe (single leaf). No D-slice.
COMPILE: device=OK (wide_x_device), submission=OK (wide_x_submission), full_link=OK (wide_x_full_link) on cann-server3 / dav-2201 / Ascend910B3.
SOURCE: /Users/sunyiyang/Desktop/Project/cann/phase4/workspaces/WIDE-X/submission.asc
COMPILE_LOG: /Users/sunyiyang/Desktop/Project/cann/phase4/workspaces/WIDE-X/logs/{configure_v001,compile_device_v001,compile_submission_v001,compile_fulllink_v001}.log (stages rebuilt in place for V002)
CLEANUP: no source deleted; server build kept at phase4-workspaces/WIDE-X/build.
ONLINE_READY: true
EXPECTED_AFFECTED_CASES: 15/15 (local golden matrix: FP16/BF16/FP32, D=64..32768, rows=1..8, unaligned D=80/511 all bad=0). Then T14 timing.

## Local golden matrix (all EXIT 0)

| case | max_abs | bad |
|---|---|---|
| 1x64 f16/f32/bf16 | 0.002 / 2e-7 / 0.016 | 0 |
| 3x64 f16/f32/bf16 | 0.002 / 2e-7 / 0.008 | 0 |
| 4x80 f16 | 0.003 | 0 |
| 2x256 f32/bf16 | 5e-7 / 0.016 | 0 |
| 2x511/512 f16 | 0.002 | 0 |
| 6x2048 f16 | 0.002 | 0 |
| 8x1024 f32/bf16 | 5e-7 / 0.016 | 0 |
| 2x4096 f16 | 0.004 | 0 |
| 1x32768 f16/f32 | 0.002 / 1e-6 | 0 |
