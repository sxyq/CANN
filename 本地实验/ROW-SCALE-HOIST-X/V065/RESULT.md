# ROW-SCALE-HOIST-X V065 Result

## Identity

- Direct Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`)
- Candidate: `9a0ffc80edcacc1225e8763a5d57e63e023cdbf9a90d79033ecc42c81b0076d0`
- Shape/dtype: FP32 `[128,3072]`, device 0, Ascend910B3
- Dispatch: `ProcessSmallFp32Batched`, non-full-tile branch
- One change: stage `gamma * invRms` into existing FP32 `xBuf_` scratch, then multiply the value row and add bias

## Gates

| Stage | Result |
|---|---|
| Compile configure/device/submission | PASS |
| Correctness Parent | PASS; matched `1.0`, max error `3.09944153e-6` |
| Correctness Candidate first invocation | FAIL; matched `0.997395833`, max error `1.98552656` |
| Correctness Candidate diagnostic rerun | PASS; matched `1.0`, max error `3.33786011e-6` |
| Local repeated invocation correctness | FAIL; matched `0.618787130`, max error `6.19191217` |
| Local | Numeric data retained, not accepted; `LOCAL_SCORE=NONE` |

The one-shot candidate result was transiently PASS on rerun, but the Local runner's 45 warmups and 32 timed invocations exposed a repeatability correctness failure. Raw timing is diagnostic only and is not promoted to a Local score.

## Local raw evidence

- Parent: 32 samples, median `19.160000 us`, mean `29.742496 us`, MAD `14.623750 us`
- Candidate: 32 samples, median `28.750000 us`, mean `31.240000 us`, MAD `11.805000 us`
- Raw samples: `local/parent-20261008T202552Z.log`, `local/candidate-20261008T202552Z.log`
- Device context: `local/device-snapshot-before-20261008T202552Z.log`, `local/device-snapshot-after-20261008T202552Z.log`

## Continuation

- `LOCAL_SCORE=NONE`
- `CURRENT_LOCAL_BEST=V026`
- V065 is not a Parent for the next performance revision.
- V061/V062/V063 remain non-Parents; Online is forbidden.
- Next action: one independent row-scale/row-level scale-hoist OFAT from exact V026, then Compile.
