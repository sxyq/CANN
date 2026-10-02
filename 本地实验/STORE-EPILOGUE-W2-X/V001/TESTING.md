# STORE-EPILOGUE-W2-X V001 build and correctness harness

## Source identity

- Candidate: `submission.asc`, SHA256 `48b9428dc2fc97c7c9d95f03ad8cec8e758c88e1197aa2328b1b1edebc018f88`.
- Direct parent fixture: `parent.asc` (STORE-EPILOGUE-X V002), SHA256 `59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839`.
- The correctness runner launches each binary once per case. It does not warm up, record elapsed time, or collect profiling data.
- Each output is compared to the existing host golden with the V002 tolerances, and the Candidate output bytes are compared exactly with the direct-parent output bytes.

## Server destination

Stage this V001 directory at the following server3 path for the device 5 run:

```text
/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001
```

Use device ID `5` for correctness. Build files go under `build/`; build command logs go under `logs/`; correctness case logs and output records go under `logs/correctness-run-001/`. The correctness script refuses to reuse an existing result directory.

## Build command

Run on server3 from the staged V001 directory. This script configures CMake and separately builds the Candidate kernel object and the two linked correctness executables. Every CMake command has a separate log; redirect the driver output to its own file as shown.

```bash
cd /home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001
mkdir -p ./logs
bash ./build_v001.sh > ./logs/build-driver.log 2>&1
```

Build logs:

- `logs/cmake-configure.log`
- `logs/build-submission-object.log`
- `logs/build-parent-correctness.log`
- `logs/build-candidate-correctness.log`
- `logs/build-driver.log`

The build script defaults to CANN toolkit `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`, SoC `Ascend910B3`, and NPU architecture `dav-2201`, matching the committed STORE V002 harness setup. Record the actual server toolkit version and SoC with the run evidence.

## Correctness command

Run after all three build targets succeed:

```bash
cd /home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001
DEVICE_ID=5 RESULT_DIR=/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-001 \
  bash ./run_correctness_v001.sh \
  > /home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-001.log 2>&1
```

The matrix contains 26 `(rows, width, dtype)` inputs; dtype codes are `0=FP32`, `1=FP16`, `2=BF16`:

- FP32 (16): `1x1024`, `1x4096`, `2x4096`, `1x8192`, `2x8192`, `1x16384`, `1x32768`, `3x6144`, `1x100`, `33x100`, `8x256`, `2x256`, `32x256`, `2x6144`, `2x8192`, `8x8192`.
- FP16 (6): `1x256`, `1x4096`, `1x8192`, `1x16384`, `1x65`, `17x257`.
- BF16 (4): `1x256`, `1x4096`, `1x8192`, `1x32768`.

Each case has separate `parent_*.log` and `candidate_*.log`, binary output, and result TSV under `logs/correctness-run-001/`; the matrix summary is `correctness-summary.tsv`. A pair passes when both golden checks pass and outputs are byte-identical. The committed STORE V002 record reports matching parent/candidate golden mismatches for `1x16384 FP32` and `1x32768 FP32`; for these two cases the script records `PARENT_GOLDEN_MISMATCH_OUTPUT_EQUAL` only when both runs return the golden-mismatch status and the output bytes still match exactly. Other golden failures or any byte difference fail the matrix.

## Current state

These are prepared commands only. No server access, build, correctness, timing, profiling, or Online action has been performed for V001.
