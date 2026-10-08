# ROW-SCALE-HOIST-X V067 Result

## Identity

- Direct Parent: exact V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`); V066 was not used as Parent.
- Candidate: `77629bd2ab28472f4f8215f2bea8ab5b6b07b41759ea989e7ea5fc3f2c2c427d`.
- Shape/dtype: FP32 `[128,3072]`; physical device 3, Ascend910B3; runner logical device 0.
- Dispatch: `ProcessSmallFp32Batched`, non-full-tile branch.
- Single change: move the existing per-row `invRms` multiplication from before gamma to after gamma and bias.

## Gates

| Stage | Result |
|---|---|
| Compile configure/device/submission | PASS; source and compile input identities match |
| Correctness Parent | PASS; matched `1.0`, max error `3.09944153e-6` |
| Correctness Candidate | FAIL; matched `0.006645203`, max error `0.0688859224`, exit 2 |
| Local | NOT RUN; Candidate correctness gate failed |

The fresh physical-device-3 snapshot showed `3430/65536 MB` HBM used (`62106 MB` free) and no NPU 3 process. Parent and Candidate were run sequentially with the same runner protocol. Both complete outputs and exit status are retained. The after snapshot shows no V067 runner process, and the explicit release receipt records device 3 released after Correctness.

## Verdict

- `CORRECTNESS_FAILED`; this is not a measurement result and has no Local score or delta.
- `CURRENT_LOCAL_BEST=V026`; V067 is not promoted and is not a Parent.
- Online is forbidden. Raw correctness output, snapshots, build logs, runner identities, and the exact one-hunk `diff.patch` remain under this directory.
