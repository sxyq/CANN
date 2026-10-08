# ROW-SCALE-HOIST-X V066 Result

## Identity

- Direct Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`).
- Candidate: `2469938d543889dd5feaac170b6c9b064952aa36e0d24fc82cc49d303783bb8d`.
- Shape/dtype: FP32 `[128,3072]`; physical device 3, Ascend910B3; runner logical device 0.
- Dispatch: `ProcessSmallFp32Batched`, non-full-tile branch.
- Single change: multiply each value row by gamma before row `invRms`, then add bias.

## Gates

| Stage | Result |
|---|---|
| Compile configure/device/submission | PASS; required route targets built |
| Compile full_link | Diagnostic failure: host object is incompatible with `elf64-littleaarch64`; log retained |
| Correctness Parent | PASS; matched `1.0`, max error `3.09944153e-6` |
| Correctness Candidate | PASS; matched `1.0`, max error `3.09944153e-6` |
| Local | 9/9 invocations exit 0; all raw samples retained |

The first correctness launch failed before device access because `libmsprofiler.so` was not on the loader path. The same binaries were rerun once after sourcing the existing toolkit environment; both gates passed. Both attempts are retained.

## Numeric Local result

- Protocol: one Parent stability invocation, then four interleaved Parent/Candidate blocks; 45 warmups and 32 device-event samples per invocation.
- Pooled samples: 128 Parent / 128 Candidate; raw total 288 samples.
- Parent pooled median/mean: `17.5700005 us` / `16.863437625 us`.
- Candidate pooled median/mean: `16.290001 us` / `17.240624984375 us`.
- Descriptive ratio-of-medians score: `107.8575777865207`.
- Candidate median delta: `-7.285142080673246%`; candidate mean delta: `+2.2367169005673038%`.
- Paired block medians: B1 `+34.29014873119274%`, B2 `-21.870496927350047%`, B3 `+21.807061079923184%`, B4 `-18.34795023201572%`; Candidate faster in `2/4` blocks.
- Parent pooled CV/MAD: `0.3959807632997036` / `5.390937531249998 us`; Candidate pooled CV/MAD: `0.5895910273426297` / `6.329062468750001 us`.

The numeric result is `MEASUREMENT_BLOCKED`: paired directions are mixed, the median improvement conflicts with a slower pooled mean, and Candidate variability is higher. The measurement is descriptive only; `CURRENT_LOCAL_BEST=V026` and V066 is not promoted. Raw logs, snapshots, executable identities, and the release receipt remain under `local/`.

## Evidence

- Raw timing: `local/PARENT_STABILITY-20261008T210334Z.log`, `local/PARENT_BLOCK1-20261008T210334Z.log` through `local/PARENT_BLOCK4-20261008T210334Z.log`, and matching Candidate logs.
- Numeric/statistical record: `local/local-result.json`.
- Before/after device snapshots: `local/device-snapshot-before-local-20261008T210211Z.log`, `local/device-snapshot-after-local-20261008T210425Z.log`.
- Release: `local/device-release.log`; physical device 3 reported no running NPU process after capture.
- Identity and gate records: `source-meta.json`, `compile-result.json`, `correctness-result.json`, and `diff.patch`.

## Continuation

- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; Online is forbidden.
- V066 is not a Parent; the next authorized sibling must use exact V026 as Direct Parent and stay within the row-scale/row-level scale placement axis.
