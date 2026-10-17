# SYNC V001 Parent Same-Binary Result

- Lease: `M1-SYNC-V001-D4-REQUALIFY-PC-W60-20261003T115153Z` (`RELEASED`)
- RUN_ID: `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-REQUALIFY-PC-W60-20261003T124637Z`
- Device/shape: d4, `[2,12288]` FP16
- Timing executable SHA256: `99236e9e0e36c6c539cfc102bf149e84ba20a89b83bddf441cb00ba9bd9d4413`
- Candidate/Parent source SHA256: `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec` / `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate/Parent module SHA256: `1a7172946ceab8786920f3e86d2df11307d58e22d86a8f6da90d11cce3844f4` / `017ab02a4fe0da2c2692ba18c17c89ad27cd928ef66a14bf8035d2c4731ec4ec`
- Runner: commit `041e653f`, `runner_main.inc` SHA256 `31f4d25b314f7a965d7bc00cc1b2625a761b6c752ad0fd4c1956a29cacd34c4a`

## Result

Parent same-binary used 60 synchronized warmups and two blocks of 31 device-event samples. Runner returned RC=1: median `18.05 us`, MAD/median `0.326870`, block drift `1.191136`; both limits are `0.10`. The block medians were `15.50 us` and `37.00 us`.

Parent correctness passed (`matched_ratio=1.0`, `max_abs=0.00048828125`). Candidate P/C was NOT_RUN. V001 Local verdict is `MEASUREMENT_BLOCKED`: same-binary stability did not qualify, so this run makes no Candidate performance finding.

## Load and Files

At `12:48:14Z`, d4 HBM was `59191/65536 MB` (90%); AICore reads were 54% in the device table and 69% in the detailed usage snapshot. VLLM PID `2999855` remained present. At `12:54:20Z`, d4 HBM was `59190/65536 MB`, AICore was 62%, and no SYNC runner remained. Project disk availability was `591 GB` at the pre-run and `12:57:35Z` captures. These readings are retained as load evidence; no other process was altered.

Remote output directory:

`/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-REQUALIFY-PC-W60-20261003T124637Z/`

Local evidence directory: `SAME-BINARY-REQUALIFY-PC-W60-20261003T124637Z/`. All 17 expected files are present locally. Local and remote SHA256 match for every copied file, including raw samples and jitter. Raw SHA256: `6b06bdd3551219d3b39695b4ad918ebe7a9e8c23f250af6fc02acebbf7c9dd72`; jitter SHA256: `801d3dbab5a3c3712806044c6844cec91c4addd83898727c94c658e0c6bf4d29`.
