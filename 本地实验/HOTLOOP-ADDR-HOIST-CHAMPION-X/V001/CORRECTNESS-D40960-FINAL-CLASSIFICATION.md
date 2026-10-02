# ADDR V001 BF16-D40960 Final Correctness Diagnostic Classification

```text
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
REVISION=V001
HOST=hwnput3
DEVICES=4,7
DEVICE_PLATFORM=Ascend910B3
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602
RUNNER_SHA256=1df859b0d8e7161ac5d95962a97b06776011153e92f82c9d7f12e7c853c38b19
BUILD_SUPPORT_COMMIT=4e045d1a9ed6d90f671cb88352cd82a7b562d8ca
CLASSIFICATION=SHARED_PATH_SIDE_EFFECT
EXACT_BLOCKER=bf16-d40960 aclrtSynchronizeStream=507035
CANDIDATE_BUG=NOT_ESTABLISHED
CORRECTNESS=INCOMPLETE
PERFORMANCE=NOT_ELIGIBLE
TIMING=FORBIDDEN
V002=FORBIDDEN
```

## Classification basis

The same required BF16 D=40960 case reaches `CASE_BEGIN` and the first
`LAUNCH_BEGIN`, then fails at `aclrtSynchronizeStream` with status `507035`
and runner return code 2. This occurs for both binaries, on both d4 and d7,
with separate fresh processes and non-interleaved launches. Therefore the
evidence establishes a shared shape/runtime/harness side effect, not a
Candidate-specific H3 bug. The exact runtime blocker is retained without
generalizing it to a kernel correctness failure.

| Device | Binary | Exact terminal output | RC | Load fact |
|---|---|---|---:|---|
| d4 | Parent-only | `CASE_BEGIN ... bf16-d40960` → `LAUNCH_BEGIN ... PARENT_R31B_V011` → `kernel synchronize failed: 507035 (null)` | 2 | HBM 93%, AICore 0% |
| d4 | Candidate-only | `CASE_BEGIN ... bf16-d40960` → `LAUNCH_BEGIN ... CANDIDATE_ADDR_V001` → `kernel synchronize failed: 507035 (null)` | 2 | HBM 93%, AICore 0% |
| d7 | Parent-only | `CASE_BEGIN ... bf16-d40960` → `LAUNCH_BEGIN ... PARENT_R31B_V011` → `kernel synchronize failed: 507035 (null)` | 2 | HBM 37% |
| d7 | Candidate-only | `CASE_BEGIN ... bf16-d40960` → `LAUNCH_BEGIN ... CANDIDATE_ADDR_V001` → `kernel synchronize failed: 507035 (null)` | 2 | HBM 37% |

The d7 complete stdout, pre/post `npu-smi`, effective system-first loader
path, `ldd`, and return code are retained in:

- `logs/correctness-d40960-parent-only-local-hwnput3-d7-20261002.txt`
- `logs/correctness-d40960-candidate-only-local-hwnput3-d7-20261002.txt`
- `CORRECTNESS-D40960-PARENT-DIAG-2.md`
- `CORRECTNESS-D40960-CANDIDATE-DIAG-2.md`

The d4 exact run facts above are the verified support diagnostic checkpoint
recorded here without rerunning or fabricating an uncollected raw npu-smi
transcript.

## Exact d4 commands

```text
set +u; source /usr/local/Ascend/ascend-toolkit/set_env.sh; set -u; export task_ld_path=/usr/lib/aarch64-linux-gnu:/usr/lib/gcc/aarch64-linux-gnu/11:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/lib64:${LD_LIBRARY_PATH:-}; export LD_LIBRARY_PATH=$task_ld_path; ./本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/build-correctness/addr_v001_correctness 4 bf16-d40960 parent
set +u; source /usr/local/Ascend/ascend-toolkit/set_env.sh; set -u; export task_ld_path=/usr/lib/aarch64-linux-gnu:/usr/lib/gcc/aarch64-linux-gnu/11:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/lib64:${LD_LIBRARY_PATH:-}; export LD_LIBRARY_PATH=$task_ld_path; ./本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/build-correctness/addr_v001_correctness 4 bf16-d40960 candidate
```

The exact d7 command form is identical with device `7`; its expanded
system-first `LD_LIBRARY_PATH` and complete stdout are in the retained d7
logs listed above. `libstdc++.so.6` resolves to
`/usr/lib/aarch64-linux-gnu/libstdc++.so.6`.

## Gate decision

The eight previously passing FP16/BF16 cases do not close the required
D40960 matrix. Since both isolated binaries fail at the same first sync point,
the final ADDR-H3 V001 correctness state is `INCOMPLETE`, classified as
`SHARED_PATH_SIDE_EFFECT` with the exact blocker above. No both/equality
closure, qualification, local timing, Online action, or V002 creation is
permitted from this evidence.

The existing `local-result.json` is intentionally not overwritten; this file
is the final route-local classification record for the diagnostic event.

