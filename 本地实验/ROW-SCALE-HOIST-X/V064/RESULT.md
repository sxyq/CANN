# ROW-SCALE-HOIST-X V064 Result

## Identity

- Direct Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`)
- Candidate: `0a7ee605bcbe2dd0120242835f3499276f0f4d37c1cd8bd255bf8d722676e4ef`
- Shape/dtype: FP32 `[128,128]`, device 0, Ascend910B3
- Dispatch: `ProcessSmallFp32ContiguousBatched`, narrow branch
- One change: gamma -> bias -> existing per-row invRms scale, only in the narrow FP32 branch

## Gates

| Stage | Result |
|---|---|
| Compile configure | PASS |
| Compile device/submission targets | PASS |
| Correctness Parent | PASS; matched `1.0`, max error `7.15255737e-7` |
| Correctness Candidate | FAIL; matched `0.0`, max error `0.699000657`, limit `0.01` |
| Local | NOT RUN; Candidate correctness gate failed |

The Candidate source delta executed in the intended branch. The post-bias scale placement is not correctness-preserving for this operator and is retained as a failed OFAT. No Local score is claimed.

## Continuation

- `LOCAL_SCORE=NONE`
- `CURRENT_LOCAL_BEST=V026`
- V064 is not a Parent for the next performance revision.
- Online is forbidden.
- Next action: one new row-scale/row-level scale-hoist OFAT from exact V026, then Compile.
