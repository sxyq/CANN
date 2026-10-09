# W5-R02 REGISTER-SPILL-ELIMINATION-X

AGENT_ID=W5-R02-REGISTER-SPILL-ELIMINATION-X
ROUTE=W5-R02-REGISTER-SPILL-ELIMINATION-X
WORKTREE=/home/data4t2/lelinfeng/cann-w5-r02-register
BRANCH=research/w5-r02-register-spill
CURRENT_STAGE=ORTHOGONAL_RESEARCH_ONLY
RECEIPT_TIME_UTC=2026-10-09T15:54:38Z

## RULE_REFRESH_RECEIPT

Read before this evidence write:

- `AGENTS.md`
- `.agents/skills/cann-route-executor/SKILL.md`
- `项目规则/实验总则.md`
- `项目规则/执行约定.md`
- `项目规则/服务器实验规范.md`
- `项目规则/本地性能测试规范.md`

Scope remained one Route, one existing worktree, and one existing branch. No
Candidate source, shared scheduler, shared score, archive, or other Route file
was edited.

## Parent And Environment

- Parent: `线上结果/R31B/V011/submission.asc`
- Parent SHA-256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Parent bytes: `190020`
- Exact historical copy: `归档/历史工作区/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc`
- Historical copy comparison: byte-identical to the parent
- Toolkit: `/usr/local/Ascend/ascend-toolkit/latest` -> `8.5.0.alpha002`
- Compiler: `bisheng`, clang `15.0.5`
- SoC: `Ascend910B3`; target: `dav-2201`
- NPU resource context: device `2`, HBM `65536 MB`, usage `5%`, free `62259 MB`, AICore `0%`
- Resource gate: satisfied; no NPU task was launched by this research probe

## Exact Source And Existing Command

The established R31B path is an ASC adapter translation unit:

```text
compile_adapter_v011.asc
  #include "R31B-V011-LP-ROW-PIPELINE_kernel.asc"
```

The exact historical compile command is retained in:

```text
/home/data4t2/lelinfeng/phase4-workspaces/R31B/build-recheck/CMakeFiles/r31b_submission_v011.dir/build.make:76
```

It uses `bisheng`, `-c -x asc`, `--cce-aicore-arch=dav-c220`, the HCC C++
include roots, and `--npu-arch=dav-2201`. The prior successful build summary is
`归档/历史工作区/R31B/compile-v011-server3.log`.

The full raw probe commands and outputs are in:

```text
研究/REGISTER-SPILL-ELIMINATION-X/compiler/champion-resource-probe-v001.log
```

## Evidence

1. The corrected established ASC command compiled the exact parent with
   `ASC_RC=0`, but `bisheng` reported `--cce-res-usage` unused.
2. Passing the `.asc` source to CCE mode reproduced the prior error:
   `cannot compile ASC file and CCE file together`.
3. Copying the exact bytes to a temporary `.cpp` input and placing all CCE
   flags before the input compiled successfully (`CCE_CPP_RC=0`). The CCE
   compiler still reported `--cce-res-usage` unused.
4. Replacing the public flag with internal `-mllvm -cce-res-usage` also
   compiled successfully but emitted no output at all.
5. The CCE object contains the three FP32/FP16/BF16 kernel symbols and a fixed
   `.ascend.stack.size.record` with two `0x8000` records. The same metadata is
   present in an existing simple AddRmsNormBias control object, so it is not
   evidence of a target-specific register spill.
6. No `Function properties`, `Stack size`, `Used register number`,
   `Total Spilled Byte Size`, `NumSpills`, `NumReloads`, scratch, or live-range
   output was obtained. The available disassembler cannot decode `elf64-hiipu`
   instructions, and `bisheng -S` is unsupported for this CCE path.

Target source locations remain observation-only:

- `ProcessWideFp32FullCacheRows` at line `2082`
- `ProcessWideLowPrecision` at line `3076`
- vector kernel entry at line `3469`

## Decision

```text
HYPOTHESIS_STATUS=FEASIBILITY_UNPROVEN
ORTHOGONALITY_STATUS=ORTHOGONAL_RESEARCH_ONLY
CANDIDATE_EDIT=NO
V001_CREATED=NO
CORRECTNESS=NOT_RUN
LOCAL_SCORE=NONE
ONLINE=NOT_SUBMITTED
```

The research does not prove a real register-spill bottleneck, so the receipt
does not authorize a one-variable live-range change. This is not a Route
lifecycle decision; Planning/Review retains that authority.

## Route Receipt

```text
ROUTE_EVENT
ROUTE=W5-R02-REGISTER-SPILL-ELIMINATION-X
REVISION=NONE
LAST_ACTION=READ_ONLY_PARENT_COMPILER_RESOURCE_PROBES
NEXT_ACTION=STOP_BEFORE_CANDIDATE_EDIT; REQUIRE_DIRECT_VECTOR_RESOURCE_EVIDENCE
CHANGE=NONE
COMPILE=PASS_READ_ONLY_PARENT_PROBE
CORRECTNESS=NOT_RUN
FREE_HBM_MB=62259
DEVICE_ID=2
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
CURRENT_LOCAL_BEST=NONE
GIT_COMMIT=SEE_COMMIT_CONTAINING_THIS_RECEIPT
PUSH=NO
BLOCKER=FEASIBILITY_UNPROVEN_NO_DIRECT_VECTOR_RESOURCE_REPORT
```

No formal Revision or `VERSION_RECORD_EVENT` is claimed because no Candidate
change was made. Next action is to hand this receipt to Main/Planning; do not
create V001 or submit Online unless a future toolchain/resource path supplies
direct vector register/spill/live-range evidence and the receipt is updated by
an authorized new action.
