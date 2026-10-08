# V001 Host/Device Repeatability Diagnostic

This support-only diagnostic is limited to rank-2 `[2,16384]` FP32. It builds separate Parent and Candidate executables from the existing V001 sources. It does not edit `submission.asc` or create a Revision.

Each invocation writes new files only. The Parent/Candidate host arrays are snapshotted immediately before their H2D copies; SHA-256 is recorded from those exact snapshots, which are the same arrays passed to H2D. After H2D synchronization, each input allocation is copied back to host and saved. After the kernel and stream synchronization, the same `deviceOutput` allocation is copied to two distinct host buffers and saved as `output_d2h_first.bin` and `output_d2h_second.bin`. The runner compares the input snapshots with their H2D readbacks and the two output copies byte-for-byte. The run script computes SHA-256 for every saved binary artifact and retains all logs, failed results, and partial artifacts.

Output files use `open(..., O_CREAT | O_EXCL, ...)`; an existing artifact is never truncated. The build script rejects an existing build path. The run script creates a unique result directory with `mkdir` and stops if that path already exists. It does not remove or replace files.

## CPU Reference

The harness checks output twice: `repeatability_main.inc` uses the current Correctness host formula, and `verify_cpu_golden.py` independently decodes the saved FP32 buffers and recomputes the result in Python double precision with `math.fsum` for the row square-sum. Inputs remain the exact host arrays captured before H2D. For each row the reference is:

```text
v[i] = double(x[i]) + residual[i]
inv_rms = 1 / sqrt(sum(v[i] * v[i]) / width + 1e-5)
expected[i] = v[i] * inv_rms * gamma[i] + bias[i]
```

This is the AddRmsNormBias equation evaluated independently of the device output bytes. Both CPU implementations apply the current criteria to both D2H copies: `atol=2^-16`, `rtol=2^-10`, matched ratio at least `0.99`, and maximum absolute error at most `1e-2`. The Python result is written separately to `cpu-golden.json`; this cross-checks the existing host formula without claiming comparison against a separate framework oracle.

## Build and Run

Run these commands on server3 only after Main assigns the device and synchronizes these support files to the exact V001 source directory. That staged directory must also contain `parent.asc` with SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`, matching the local R31B V011 source and its `submission.sha256` sidecar; the build script checks it alongside `submission.asc`. Use one `RUN_ID` for the new build and result paths. `mkdir` in each script rejects a path that already exists.

```bash
cd /home/data4t2/lelinfeng/cann/server_runs/SELECTIVE-FASTPATH-CHAMPION-X/V001-fe4f6116
RUN_ID="$(date -u +%Y%m%dT%H%M%S.%N)-$$"
BUILD_DIR="$PWD/support/build-repeatability-$RUN_ID"
RESULT_DIR="$PWD/support/results/repeatability-$RUN_ID"
bash support/repeatability/build_repeatability.sh "$BUILD_DIR"
bash support/repeatability/run_repeatability.sh "$BUILD_DIR" "$RESULT_DIR" 2
```

The build command verifies the exact Candidate and Parent source digests before creating the two diagnostic executables and logs. It records the executable, object, and support-source digests. The run command rechecks the sources and all recorded digests, then makes four sequential invocations in this order: Parent rep1, Candidate rep1, Parent rep2, Candidate rep2. Each invocation has its own directory below `RESULT_DIR`. A nonzero runner or Python verifier return does not stop later calls; all files that were produced remain in place.

Do not use `support/run_correctness.sh` for this diagnostic.
