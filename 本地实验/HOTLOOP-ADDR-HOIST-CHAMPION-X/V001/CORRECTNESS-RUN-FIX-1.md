# V001 Correctness Run Fix 1 — INCOMPLETE (D40960 Sync 507035)

```text
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
REVISION=V001
HOST=hwnput3
DEVICE=7
DEVICE_PLATFORM=Ascend910B3
DATE_UTC=2026-10-02T17:46:40Z
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602
RUNNER_SHA256=fe4607229e2a962e5d2d39dcc4b510cb13d7cf0b6ce265bf875c470e4eb3f770
```

## Result

```text
RUNTIME_ENV=PASS (system libstdc++ selected; runner entered ACL)
FP16_D12288=Parent PASS, Candidate PASS, exact output match
FP16_D16384=Parent PASS, Candidate PASS, exact output match
FP16_D24576=Parent PASS, Candidate PASS, exact output match
FP16_D32768=Parent PASS, Candidate PASS, exact output match
BF16_D12288=Parent PASS, Candidate PASS, exact output match
BF16_D16384=Parent PASS, Candidate PASS, exact output match
BF16_D24576=Parent PASS, Candidate PASS, exact output match
BF16_D32768=Parent PASS, Candidate PASS, exact output match; tile=2560, tail=2048
BF16_D40960=NOT_COMPLETE; aclrtSynchronizeStream=507035 on the next case
CORRECTNESS=INCOMPLETE
RUNNER_RC=2
LOCAL_PERFORMANCE=NOT_STARTED
```

The current runner did not label the binary immediately before each launch;
the `507035` occurred after the BF16 D=32768 case and before its D=40960
case completed. The first eight cases are independently reported as Parent
and Candidate PASS with zero exact-output mismatches. A bounded support-only
diagnostic will add launch labels and a single-case filter to identify whether
the D40960 sync fault is parent, candidate, or device/runtime state. This is
not yet a Candidate correctness verdict.

The d7 HBM snapshot stayed at roughly 24789/65536 MB used and the existing
`python3` process remained present; no process was changed. No timing,
qualification, or performance action was started.

## Exact command and evidence

```text
set +u; source /usr/local/Ascend/ascend-toolkit/set_env.sh; set -u
export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:/usr/lib/gcc/aarch64-linux-gnu/11:<CANN_RUNTIME_LIBS>:<inherited_runtime_libs>
./本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/build-correctness/addr_v001_correctness 7
```

Log:

- `logs/correctness-run-fix1-local-hwnput3-d7-20261002.txt`
