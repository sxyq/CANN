# STORE-EPILOGUE-W2-X V001 Correctness Run 002

STAGE_STATUS=FAIL
HARNESS_RC=1
RUN_START=2026-10-02T15:03:18+00:00
RUN_END=2026-10-02T15:06:54+00:00

## Identity

- Route checkout commit: `5b632c4da540cb3d711d72c1c2b9843bae759eba`
- Candidate source commit: `a1ebfc125ce215fc81ffbe04f582d5f8682e3294`
- Candidate `submission.asc` SHA256: `48b9428dc2fc97c7c9d95f03ad8cec8e758c88e1197aa2328b1b1edebc018f88`
- Direct Parent `parent.asc` SHA256: `59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839`
- Correctness harness SHA256: `7f01f2af23c273d56c3b39770b944be9b65f6f2a4d2ebea649ac0921f1f79eb2`
- Parent executable SHA256: `a55a4859dd22970216405826c7cb7f37fef4678c491979631505b0aac3f573a7`
- Candidate executable SHA256: `469522ba548c001e4ba223eb069d170fa0e73a8285f9f5aaaab696a5edd45328`

## Runtime Path Diagnosis

- `libgraph.so` real path: `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libgraph.so`
- Both executables had unresolved Ascend libraries with the previous shell environment.
- Setting `LD_LIBRARY_PATH` to `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64` resolved all `ldd` dependencies for both executables.
- The harness now prepends that directory and preserves existing `LD_LIBRARY_PATH` entries.

## Run Command and Outcome

```sh
cd /home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001
DEVICE_ID=5 RESULT_DIR=/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-002 \
  bash ./run_correctness_v001.sh \
  > /home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-002.log 2>&1
```

- Cases: `25`; unique keys: `25`; duplicates: `0`
- PASS: `23`
- FAIL: `2`
- Runtime loader errors: `0`
- `1x16384 FP32`: Parent RC `3`, Candidate RC `3`; Parent max_abs `1.09992654`, bad `14328`; Candidate max_abs `1.25959797`, bad `9081`; output bytes differ.
- `1x32768 FP32`: Parent RC `3`, Candidate RC `3`; both max_abs `1.20309962`; Parent bad `24441`, Candidate bad `28017`; output bytes differ.
- These are the two known Parent/golden mismatch FP32 shapes. The current runner permits this known mismatch only when Parent and Candidate output bytes are equal; both cases fail that condition here. This run does not establish correctness against an independent golden for these two inputs.
- No timing was run. No Candidate, executable, system library, or device process was changed.

## Server and Resource Evidence

- Host: `hwnput3`; device: `5`; toolkit: `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`; SoC: `Ascend910B3`; `npu-smi`: `25.0.rc1.1`.
- Route checkout found dynamically with `git worktree list`: `/home/data4t2/lelinfeng/cann-w2-m1-store`, HEAD `5b632c4da540cb3d711d72c1c2b9843bae759eba`.
- Before run at `2026-10-02T15:02:14+00:00`, device 5 HBM was `64050/65536 MB` (free `1486 MB`), AICore `0%`; listed processes included PID `89602` `VLLMWorker_TP` and PID `1819588` `python`.
- After run, `npu-smi` showed device 5 HBM `61443/65536 MB` (free `4093 MB`), AICore `0%`; listed processes included PID `89602` `VLLMWorker_TP` and PID `2771372` `python`. No process was stopped or migrated by this run.
- Available disk before and after run: `629 GB`.

## Logs and Failed Outputs

- Driver: `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-002.log` (SHA256 `cdadbf20047d1356696d3c2cd6a650030d6c58fe6999da06069a4ff7ff9ebeb6`)
- Summary: `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-002/correctness-summary.tsv` (SHA256 `4a27c53ebf6fd2b7856df350d7ff1475cdcf38976aba316f4744b776899fdc25`)
- All per-case Parent/Candidate logs and TSV records: `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-002/`
- Failed output binaries are retained:
  - `parent_r1_d16384_t0.bin`: `882e8207d91a0a93da623efea6ebc7ca803cde732a55ce31ec3c0b95755367b9`
  - `candidate_r1_d16384_t0.bin`: `264e87d9aa3104d8636b9a02341677558258576cda7ac515713f9721045e4a31`
  - `parent_r1_d32768_t0.bin`: `7a076a2c2f909b0bb6fc21919902981c040bdb5a519a8dfa5abcf8c32af0285e`
  - `candidate_r1_d32768_t0.bin`: `b0dd95913e645ceabc88b980ebf098e76b3dc6c9df3653db89ef1da3b2465987`

## Run 001 Preservation

- Run 001 remains at `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-001/` with driver log `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-001.log`.
- Before and after run 002, run 001 contained `51` files with directory digest `b6329a631ddf59f51e9ae03be9de3a45aedab6b0649334c06d542e95522604d8`; its driver log SHA256 remained `33c142bc4c2226e1906ec8839553f7e0b29fe4162ae0b3b536f88c6c23e9a15e`.
