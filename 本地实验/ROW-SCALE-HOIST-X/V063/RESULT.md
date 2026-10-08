# ROW-SCALE-HOIST-X V063 Result

## Identity

- Direct Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`)
- Candidate: `dfeacc52a5e8d1283ecc2c48a7a76c10fb803caadb860cd372891e04fbf68747`
- Shape/dtype: FP32 `[128,128]`, device 0, Ascend910B3
- Dispatch: `ProcessSmallFp32ContiguousBatched`, narrow branch
- One change: apply gamma, then the existing per-row invRms scale, then bias

## Gates

| Stage | Result |
|---|---|
| Compile configure | PASS |
| Compile device/submission targets | PASS |
| Compile full_link diagnostic | FAIL: host object incompatible with `elf64-littleaarch64`; retained, not a candidate failure |
| Correctness Parent | PASS; matched `1.0`, max error `7.15255737e-7` |
| Correctness Candidate | PASS; matched `1.0`, max error `7.15255737e-7` |
| Local invocations | PASS; 9/9 exit 0 |

## Local measurement

- Device-event timing, 45 warmups, 32 samples per invocation.
- One Parent stability invocation plus four interleaved Parent/Candidate blocks.
- Pooled Parent: median `15.29 us`, mean `18.6685939141 us`, CV `0.7587666707`.
- Pooled Candidate: median `19.20 us`, mean `20.2582811016 us`, CV `0.8436038439`.
- Throughput: Parent `1071550032.7011`, Candidate `853333333.3333` elements/s; delta `-20.3645833333%`.
- Paired direction: Candidate faster in `2/4` blocks.
- Numeric descriptive Local score: `79.6354166667`.
- Latency delta `(candidate - parent) / parent`: `+25.5722694572%`.

The timing is `MEASUREMENT_BLOCKED`: the paired direction is mixed, variability is high, and concurrent device activity was preserved as context. The score is numeric but descriptive only; `CURRENT_LOCAL_BEST` remains V026. V063 is not a valid Parent for the next performance revision.

The initial missing-build-directory Local attempt is retained under `local/failed-attempt-missing-build-dir-20261008T193745Z/`. The initial missing-environment Correctness attempt is retained under `correctness/`.

## Continuation

- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`
- `ONLINE_FORBIDDEN=YES`
- Next revision must use exact V026 as Direct Parent.
- Next action after this evidence commit: one new row-scale placement OFAT, then Compile.
