# Code Footprint Read-Only Tooling Check

AGENT_ID = NOT_ASSIGNED_BY_MAIN (not invented)
ROUTE = CODE-FOOTPRINT-ICACHE-X
BRANCH = research/w5-r03-code-footprint
WORKTREE = /home/data4t2/lelinfeng/cann-w5-r03-icache
CURRENT_STAGE = READ_ONLY_FEASIBILITY
CHECKED_AT = 2026-10-09T15:46:35+00:00
CURRENT_REVISION = NONE (V001 not created)

## Scope and preservation

This receipt records read-only checks only. No Candidate source, shared scheduler,
shared score record, W4 archive, Main-1 file, other route file, device task, or
Online submission was modified. The existing untracked files in this route
directory were preserved unchanged.

Required rule and route context was read from:

- `AGENTS.md`
- `.agents/skills/cann-mainline/SKILL.md`
- `.agents/skills/cann-route-executor/SKILL.md`
- `项目规则/实验总则.md`
- `项目规则/执行约定.md`
- `项目规则/服务器实验规范.md`
- `项目规则/本地性能测试规范.md`
- `项目规则/线上提交规范.md`
- `项目规则/Git工作流程.md`
- `技术路线/技术路线总表.md`
- `技术路线/技术路线图.md`
- `技术路线/路线成绩表.tsv`
- `技术路线/全版本记录.tsv`
- `研究/CODE-FOOTPRINT-ICACHE-X/FEASIBILITY_RECEIPT_R3.md`
- `研究/CODE-FOOTPRINT-ICACHE-X/MAIN_STATUS_RECEIPT.md`
- `研究/CODE-FOOTPRINT-ICACHE-X/ORTHOGONALITY_RECEIPT.md`

## Server and toolchain check

The existing server entry was checked without changing SSH configuration or
remote files:

```text
SERVER_ALIAS = cann-server3
SSH_COMMAND = ssh -o BatchMode=yes -o ConnectTimeout=15 -o StrictHostKeyChecking=yes cann-server3 true
SSH_RC = 255
SSH_RESULT = Could not resolve hostname cann-server3: Temporary failure in name resolution
GETENT_RESULT = no address returned
REMOTE_MUTATION = NONE
```

The tools are not resolved from the current `PATH`:

```text
PATH bisheng = NOT_FOUND
PATH clang++ = NOT_FOUND
PATH llvm-objdump = NOT_FOUND
```

The installed target toolchain is present at absolute paths and was queried
read-only:

```text
BISHENG = /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/ccec_compiler/bin/bisheng
BISHENG_VERSION = clang 15.0.5 (clang-5c68a1cb1231 flang-5c68a1cb1231)
LLVM_OBJDUMP = /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/ccec_compiler/bin/llvm-objdump
LLVM_OBJDUMP_VERSION = LLVM 15.0.5; default target aarch64-unknown-linux-gnu; host CPU tsv110
STANDALONE_CLANGXX = NOT_FOUND at PATH and checked toolkit compiler paths
```

Host `objdump` was not used as target-code evidence.

## Exact Champion source and retained project output

The retained project reproduction is outside this Git worktree at
`/home/data4t2/lelinfeng/phase4-review-repro/R31B-V011`. Its source compares
byte-for-byte with the repository-preserved Champion source:

```text
REPOSITORY_SOURCE = 线上结果/R31B/V011/submission.asc
PROJECT_SOURCE = /home/data4t2/lelinfeng/phase4-review-repro/R31B-V011/src/submission.asc
SOURCE_SIZE = 190020 bytes
SOURCE_SHA256 = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
SOURCE_CMP = PASS
SOC = Ascend910B3
NPU_ARCH = dav-2201
```

Read-only artifact identity:

```text
OBJECT = /home/data4t2/lelinfeng/phase4-review-repro/R31B-V011/build/CMakeFiles/device.dir/src/device.cpp.o
OBJECT_SIZE = 114520 bytes
OBJECT_SHA256 = 0b96bad5944be5e88b483af4181df8c303890a0129707230454e4dad4f06b072
OBJECT_FILE = ELF 64-bit LSB relocatable, target format elf64-hiipu

LINKED_DEVICE = /home/data4t2/lelinfeng/phase4-review-repro/R31B-V011/build/device.alink
LINKED_DEVICE_SIZE = 114168 bytes
LINKED_DEVICE_SHA256 = b414e2a272004ed3e24d508d31fb264d5658576110860d099b95d88efa99524a
LINKED_DEVICE_FILE = ELF 64-bit LSB executable, target format elf64-hiipu
```

The target `llvm-objdump -h` output provides parent-only section-size evidence:

```text
OBJECT FP32 kernel text section = 0x34c8 = 13512 bytes
OBJECT FP16 kernel text section = 0x3740 = 14144 bytes
OBJECT BF16 kernel text section = 0x3a18 = 14872 bytes
LINKED .text = 0xa620 = 42528 bytes
LINKED .rodata = 0x000c = 12 bytes
LINKED .data = 0x10000 = 65536 bytes
```

Target `llvm-objdump -t` identifies the three dtype kernel symbols, but
target `llvm-objdump -d` emits `<not available>` for the `elf64-hiipu`
instructions. Therefore there is no usable instruction sequence, hot/cold
layout, or instruction-fetch report in this check. No `.s`, `.asm`, `.dis`,
code-size report, or layout report exists beside the V011 build output. The
retained compiler records are `cfg.log`, `device.log`, `submission.log`, and
`link.log`; they record PASS and the following target configuration:

```text
COMPILER = /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/ccec_compiler/bin/bisheng
COMPILE_FLAGS = -O3 --cce-aicore-lang --cce-aicore-arch=dav-c220-vec --cce-aicore-only --cce-auto-sync --npu-arch=dav-2201 --npu-soc=Ascend910B3 -std=c++17
LINKER = /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/tools/ccec_compiler/bin/ld.lld
```

## Gate decision

```text
ORTHOGONALITY = PASS (existing receipt)
PARENT_TARGET_ARTIFACT = FOUND (read-only baseline only)
TARGET_SECTION_SIZE_EVIDENCE = PARTIAL (parent-only)
TARGET_INSTRUCTION_DISASSEMBLY = UNAVAILABLE (decoder reports <not available>)
SAME_PATH_BEFORE_AFTER_COMPARISON = NOT_FOUND
GENERATED_CODE_CHANGE_PROVEN = NO
FALSIFIABLE_COMPILER_LAYOUT_EFFECT = NOT_PROVEN
FEASIBILITY_STATUS = UNPROVEN / TOOLING_ARTIFACT_BLOCKED
CANDIDATE_EDIT = NOT_STARTED
V001 = NOT_CREATED
COMPILE = NOT_RUN
CORRECTNESS = NOT_RUN
LOCAL = NOT_RUN
DEVICE_EXPERIMENT = NOT_STARTED
ONLINE = NOT_SUBMITTED
```

The V011 object and linked image prove that a target artifact can be retained
and section sizes can be inspected, but they are only one parent snapshot.
No artifact compares the exact same source and execution path before and after
a compiler hot/cold, inlining/outlining, or section-layout control. Other
source revisions are not admissible proof of that effect. No OFAT is proposed.

## Next action and blocker

NEXT_ACTION = Recover the existing `cann-server3` name resolution or obtain a
read-only paired target artifact/disassembly/code-layout report for the exact
V011 path. Only a same-path generated-code difference may authorize proposing
one compiler/layout OFAT; otherwise keep this route research-only.

BLOCKER = TOOLING_ARTIFACT_BLOCKED / FEASIBILITY_UNPROVEN: server3 is not
resolvable, target instruction decoding is unavailable, and no same-path
before/after generated-code evidence exists.

LAST_COMPLETED_ACTION = Read-only toolchain, project-output, compiler-log,
object, linked-image, and target-section inspection completed.
