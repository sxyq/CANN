# V001 Correctness Run Attempt 1 — INCOMPLETE (Loader Blocker)

```text
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
REVISION=V001
HOST=hwnput3
DEVICE=7
DEVICE_PLATFORM=Ascend910B3
DATE_UTC=2026-10-02T17:42:50Z
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602
RUNNER_SHA256=fe4607229e2a962e5d2d39dcc4b510cb13d7cf0b6ce265bf875c470e4eb3f770
```

## Result

```text
RUNNER_STARTED=NO
ACL_INIT=NOT_REACHED
NPU_KERNEL_LAUNCH=NOT_REACHED
PARENT_CORRECTNESS=NOT_RUN
CANDIDATE_CORRECTNESS=NOT_RUN
CORRECTNESS=INCOMPLETE
RUNNER_RC=1
```

The executable exited before `main()` because the dynamic loader selected the
toolkit HCC `libstdc++.so.6`, which does not provide `GLIBCXX_3.4.29` required
by the host executable:

```text
addr_v001_correctness: /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/lib64/libstdc++.so.6: version `GLIBCXX_3.4.29' not found
```

This is a host runner environment issue, not a correctness result and not a
Candidate source failure. The before/after device snapshots show d7 at
24789/65536 MB HBM used with the existing `python3` process; no NPU kernel
was launched and no process was changed.

## Exact command and evidence

```text
set +u; source /usr/local/Ascend/ascend-toolkit/set_env.sh; set -u
./本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/build-correctness/addr_v001_correctness 7
```

Log:

- `logs/correctness-run-local-hwnput3-d7-20261002.txt`

The bounded retry is the same executable with system `libstdc++` directories
placed before the toolkit runtime directories. No source or H3 change is part
of this retry.
