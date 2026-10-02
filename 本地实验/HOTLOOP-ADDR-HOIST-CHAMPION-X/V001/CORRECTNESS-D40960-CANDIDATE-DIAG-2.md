# V001 BF16-D40960 Candidate-only Diagnostic 2 — RUNTIME/HARNESS BLOCKER

```text
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
REVISION=V001
HOST=hwnput3
DEVICE=7
DEVICE_PLATFORM=Ascend910B3
CASE=bf16-d40960
BINARY_FILTER=candidate
PROCESS_MODE=fresh-exec
DATE_UTC=2026-10-02
SUPPORT_COMMIT=4e045d1a9ed6d90f671cb88352cd82a7b562d8ca
EXECUTABLE_SHA256=1df859b0d8e7161ac5d95962a97b06776011153e92f82c9d7f12e7c853c38b19
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602
```

## Result

```text
ACL_INITIALIZE=PASS
ACL_SET_DEVICE=PASS
ACL_CREATE_STREAM=PASS
KERNEL_LAUNCH=ENTERED
ACL_SYNCHRONIZE_STREAM=507035
RUNNER_RC=2
CORRECTNESS=INCOMPLETE
CLASSIFICATION=RUNTIME_HARNESS_BLOCKER
LOCAL_PERFORMANCE=NOT_STARTED
```

The Candidate-only fresh process reaches the D40960 launch and fails at
`aclrtSynchronizeStream`, before output comparison. Parent-only has the same
failure under the same concrete environment, so this is not a Candidate-only
H3 correctness failure.

## Exact command and full evidence

The command uses the requested concrete `task_ld_path` assignment, with the
post-expansion effective path printed in the log.

- logs/correctness-d40960-candidate-only-local-hwnput3-d7-20261002.txt

No H3 source, parent source, correctness arithmetic, or timing code changed.

