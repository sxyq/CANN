# SMALLMID V001 Build and Correctness Harness

## Scope

- Candidate: `../submission.asc`; expected SHA256: `a689e5abc03d2770b277812d9a52ae0a1aaf910952b737525e1722f06bdb340a`.
- Build and correctness scripts print and compare the staged source SHA256 with this expected value before proceeding, so a wrong file copied into the staging directory cannot be reported as V001 evidence.
- Device: server3 device `1`; SoC/toolchain selection follows the server environment recorded for this run.
- Cases: BF16 with `(rows, D) = (2 * vector_core_count, 2049)`, `(2 * vector_core_count, 3073)`, and `(2 * vector_core_count, 4095)`.
- The runner queries `ACL_DEV_ATTR_VECTOR_CORE_NUM`, passes that value to `run_kernel`, and uses twice that count for rows. The V001 launch path therefore keeps `block_count=vector_core_count` and reaches the existing `local_rows=2` path. No row ownership or kernel scheduling fields are changed.
- Each case launches the candidate once, synchronizes, copies the result back, and compares it with a CPU double-precision golden. It has no warmup/repeat loop, timer, performance output, or profiler call.
- BF16 acceptance: `atol=2^-6`, `rtol=2^-6`, matched ratio at least `0.99`, and maximum absolute error at most `1.0`, following `ops-precision-standard/references/float_compute.md`.

## Server Paths

Stage `submission.asc` and this `support/` directory together under a new run directory:

```text
/home/data4t2/lelinfeng/cann/local-experiments/SMALLMID-DATAFLOW-CHAMPION-X/V001/runs/<RUN_ID>/
```

Use a unique `RUN_ID` for every attempt. Build files go under that run's `artifacts/build/`; preserve the original source package and earlier attempt directories. The ASC compilation unit contains the fixed Candidate; a separate AArch64 C++ host driver calls its C ABI entry point so ordinary ACL device pointers are not cast to the ASC `__gm__` address space. Both scripts load the selected CANN environment before use. The two command logs remain separate:

```text
<RUN_ID>/build.log
<RUN_ID>/correctness.log
```

## Commands

Run on server3 from the staged V001 directory. Each command writes only its own log file.

```bash
SMD_RUN=/home/data4t2/lelinfeng/cann/local-experiments/SMALLMID-DATAFLOW-CHAMPION-X/V001/runs/<RUN_ID>
SMD_OUT="$SMD_RUN/artifacts"
mkdir -p "$SMD_RUN"
cd "$SMD_RUN"
SMD_V001_OUTPUT="$SMD_OUT" bash support/build_server3.sh > "$SMD_RUN/build.log" 2>&1
DEVICE_ID=1 SMD_V001_OUTPUT="$SMD_OUT" bash support/run_correctness_server3.sh > "$SMD_RUN/correctness.log" 2>&1
```

The build script configures the staged `support/CMakeLists.txt` and builds target `smd_v001_correctness`. The correctness script runs exactly the three BF16 cases listed above. A nonzero command exit or any `result=FAIL` is a failed correctness run; no performance conclusion is produced by this harness. Never reuse an output directory: this prevents a prior executable from being mistaken for the current attempt.
