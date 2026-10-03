# HBH-09 V002 — Runner Environment Recovery

```text
DIAGNOSIS_UTC=2026-10-03T06:51:46Z
FAILED_ATTEMPT_COMMIT=9d8e1e42
FAILURE=libruntime.so_missing_from_loader_path; runner exited 127 before main
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
```

The login command environment did not define `ASCEND_HOME_PATH` or
`LD_LIBRARY_PATH`. `/usr/local/Ascend/ascend-toolkit/set_env.sh` initializes
the standard CANN environment, but this installation keeps runtime libraries
under the versioned architecture directory, which that script did not add to
the loader path. The support-only retry invocation therefore uses:

```bash
source /usr/local/Ascend/ascend-toolkit/set_env.sh
export LD_LIBRARY_PATH="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:$LD_LIBRARY_PATH"
```

`ldd` on the exact existing performance runner then resolved all dependencies
(`LDD_RC=0`, `UNRESOLVED_LIBRARY_COUNT=0`). The complete resolution output is
`support/runner-env-recovery-20261003/ldd-after-env.log`.

This fixes only the per-command runtime environment. It does not rebuild or
modify the runner, Candidate Kernel, Parent, timing boundary, shape, or dtype.
The Window B d4 FP32 D6144 rows=80 attempt will be repeated with the same
predeclared qualification/P-C protocol and exact SHA identities. Qualification
must pass before paired timing; P/C remains on the same device under one lease.
