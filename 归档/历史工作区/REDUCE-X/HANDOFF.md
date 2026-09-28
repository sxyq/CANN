# REDUCE-X V003 HANDOFF

ROUTE: REDUCE-X
REVISION: V003
HYPOTHESIS: V001/V002 WA came from non-textbook paths (Brcb overflow, possible std::sqrt-on-device, mixed batch/zero-VS). A minimal two-pass GetValue golden for ALL shapes will hit 15/15 before any reduction experiment returns.
CHANGED: ONLY textbook golden. Two-pass over D (tile 2048). Pass1: u=x+res FP32, ReduceSum(u*u) into V accumulator, Muls/Adds mean+eps, Rsqrt, ONE GetValue of inv. Pass2: u=x+res, Muls inv, *gamma+bias, store. No Brcb. No hierarchical. No zero-VS. No batch. Launch `<<<coreNum, nullptr, stream>>>`. Skip numTensors gate.
COMPILE: device compile PASS; submission compile PASS; full link PASS (cann-server3, Ascend910B3, dav-2201, CANN 8.5.0.alpha002)
SOURCE: /Users/sunyiyang/Desktop/Project/cann/phase4/workspaces/REDUCE-X/submission.asc
COMPILE_LOG: /Users/sunyiyang/Desktop/Project/cann/phase4/workspaces/REDUCE-X/compile.log
CLEANUP: none
ONLINE_READY: yes
EXPECTED_AFFECTED_CASES: all 15
REDUCTION_DESIGN: textbook ReduceSum + GetValue once per row (inv scalar); two-pass D tiles; FP32 intermediate; CAST_NONE up / CAST_RINT down
V_S_HANDOFF_COUNT_PER_ROW: 1

## Formula

```text
u      = x + residual          (FP32)
sum    = ReduceSum(u*u)
mean   = sum / D + eps
inv    = Rsqrt(mean)           // 1/rms
output = u * inv * gamma + bias
```

## Preflight

- 5/5 PASS
- SHA-256: 74446ed229d6c155c07271cb788dbe7ff57cab9284b19f0621dc4b787389e359
- 行数 342 / 字节 11206

## Next experiment

After online 15/15: re-introduce ONE reduction idea (multi-row batch ReduceSum with aligned slots + single SyncVToS + B GetValue) and re-score.
