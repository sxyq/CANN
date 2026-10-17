# SYNC-TOPOLOGY-CHAMPION-X V001 Correctness PASS

## Run identity

- `RUN_ID`: `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-6e650cf5-20261002T231909Z`
- Route HEAD: `6e650cf5224ca0750d9d4886f5ccc428a9edde19`
- Candidate SHA256: `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`
- Direct Parent: `R31B V011`
- Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Correctness executable SHA256: `7a8335eec7156cf695cf75281a576a2b403e7df5da24de7fd7c1d8759abc97a7`

## Result

- Correctness return code: `0`.
- `[2,12288]` FP16: `PASS`, matched ratio `1.0`, maximum absolute error `0.00048828125`.
- `[2,8192]` FP16 control: `PASS`, matched ratio `1.0`, maximum absolute error `0.00048828125`.
- Both cases used the Candidate built in the same RUN_ID above.
- No timing mode was run as part of this correctness execution.

## Device evidence

- Device: d4; HBM before and after: `59190/65536 MB` used.
- AICore before and after: `0%`.
- `VLLMEngineCor` PID `2999855` remained unchanged; no SYNC process remained after execution.
- Available disk: `616 GB`.

## Server evidence

- Result directory: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-6e650cf5-20261002T231909Z`
- Correctness output: `.../correctness.log`
- Return-code record: `.../correctness-status.txt`
- Device snapshots: `.../pre-correctness-device.txt`, `.../post-correctness-device.txt`
- Process snapshots: `.../pre-correctness-processes.txt`, `.../post-correctness-processes.txt`

No Candidate/Parent source, score, or shared scheduling record was modified for this result.
