# R3 Authorized Target Layout and Analyzer Check

AGENT_ID = Parfit / W5-R03
ROUTE = CODE-FOOTPRINT-ICACHE-X
BRANCH = research/w5-r03-code-footprint
WORKTREE = /home/data4t2/lelinfeng/cann-w5-r03-icache
CHECKED_AT = 2026-10-10T13:35:54+00:00
RULES_BASE = origin/main, refreshed from commit 1c51a5b2 (public rules only)
CURRENT_REVISION = NONE (V001 not created)
CURRENT_STAGE = TARGET_ARTIFACT_LAYOUT_ANALYZER_CHECK

## Scope and W5 constraints

Read-only analysis used only the pre-existing target object and linked image
referenced by the R3 tooling receipt. Their source/object bytes were not used
as a donor, copied, or edited. No other route source/worktree, shared tooling,
W4, Main-1, Champion implementation, runner, compiler system, Candidate, or
Revision was created or changed. The failed default-architecture disassembly
probe was not repeated.

The current W5 user constraints remain in force: continue the five existing
routes/contexts; require effective patch similarity <=60% and no mechanism
overlap for any future patch; do not switch routes or use the highest-score
Champion as a donor. No patch exists in this check, so similarity is
NOT_APPLICABLE and no overlap decision for a patch is claimed.

## Existing artifact identity

These paths were already authorized/referenced by the R3 route receipt and
were only stat'ed, identified, and hashed:

```text
SOURCE = /home/data4t2/lelinfeng/phase4-review-repro/R31B-V011/src/submission.asc
SOURCE_SIZE = 190020 bytes
SOURCE_SHA256 = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

OBJECT = /home/data4t2/lelinfeng/phase4-review-repro/R31B-V011/build/CMakeFiles/device.dir/src/device.cpp.o
OBJECT_SIZE = 114520 bytes
OBJECT_SHA256 = 0b96bad5944be5e88b483af4181df8c303890a0129707230454e4dad4f06b072
OBJECT_FORMAT = ELF 64-bit relocatable; LLVM identifies elf64-hiipu

LINKED_DEVICE = /home/data4t2/lelinfeng/phase4-review-repro/R31B-V011/build/device.alink
LINKED_DEVICE_SIZE = 114168 bytes
LINKED_DEVICE_SHA256 = b414e2a272004ed3e24d508d31fb264d5658576110860d099b95d88efa99524a
```

The source SHA matches the R3 receipt's recorded source identity. This is a
pre-existing baseline artifact for analyzer/layout validation only, not a
source donor or a new implementation.

## Tool capability and exact results

Tool paths/versions:

```text
LLVM_OBJDUMP = /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/ccec_compiler/bin/llvm-objdump
LLVM_VERSION = 15.0.5
REGISTERED_TARGET = hiipu64 (HiSilicon PTC 64-bit)

MSOBJDUMP = /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/tools/msobjdump/msobjdump
MSOBJDUMP_PACKAGE = 0.1.0
MSOBJDUMP_INTERFACE = ELF/metadata/section/symbol inspection; wrapper invokes Python module

TARGET_READELF = /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/bin/readelf
TARGET_READELF_VERSION = GNU readelf (Do-Compiler V100R001C30B0032) 2.41
```

The new, explicitly targeted decoder command was:

```text
llvm-objdump --arch-name=hiipu64 --disassemble-aicore \
  --disassemble-symbols=_Z24add_rms_norm_bias_customIfEvPhS0_S0_S0_S0_mmjff \
  /home/data4t2/lelinfeng/phase4-review-repro/R31B-V011/build/CMakeFiles/device.dir/src/device.cpp.o
```

Return code was 0. LLVM recognized `file format elf64-hiipu`, the `.text.<kernel>`
section, and the function symbol, but every sampled instruction address was
reported as `<not available>`. Thus the installed LLVM advertises `hiipu64`
but does not decode the instruction payload in this artifact. No further
disassembly variant was attempted.

The existing bundled msobjdump was invoked with its package paths explicitly
available:

```text
PYTHONPATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/python/site-packages:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/python/site-packages \
PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/bin:$PATH \
/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/tools/msobjdump/msobjdump \
  --verbose --dump-elf <OBJECT> --out-dir /home/data4t2/lelinfeng/cann-w5-r03-icache
```

Return code was 0. It identified three AIV kernel metadata entries (FP32,
FP16, BF16). Inspection of the bundled module confirms it obtains symbols and
section headers via `readelf`; it is not an instruction decoder.

## Target section/symbol layout evidence

Command family, all return code 0:

```text
<TARGET_READELF> -SW <OBJECT>
<TARGET_READELF> -sW <OBJECT>
<TARGET_READELF> -SW <LINKED_DEVICE>
```

Object function text sections and global FUNC symbol sizes:

```text
FP32: .text._Z24add_rms_norm_bias_customIfEvPhS0_S0_S0_S0_mmjff
      size 0x34c8 = 13512 bytes; offset 0x010108; flags AXG
FP16: .text._Z24add_rms_norm_bias_customIDhEvPhS0_S0_S0_S0_mmjff
      size 0x3740 = 14144 bytes; offset 0x0135d0; flags AXG
BF16: .text._Z24add_rms_norm_bias_customIu6__bf16EvPhS0_S0_S0_S0_mmjff
      size 0x3a18 = 14872 bytes; offset 0x016d10; flags AXG
```

Linked image:

```text
.text   address 0x0000, size 0xa620 = 42528 bytes, flags AX
.rodata address 0xa620, size 0x000c = 12 bytes
.data   address 0xb000, size 0x10000 = 65536 bytes, flags WA
```

These are genuine target section and function-size facts for one pre-existing
parent artifact. They are not instruction-sequence, cache-miss, fetch, or
hotness evidence. No same-path before/after generated-code pair exists, and no
instruction-side profiler capture was found in the R3-owned evidence directory;
symbol size cannot be treated as a hotness measurement.

## Build gate and outcome

The current R3-owned research/local-experiment paths still contain no legal
historical build entry or input. No rebuild was started and no new artifact
was produced:

```text
NEW_SOURCE_SHA = NONE
NEW_ARTIFACT_SHA = NONE
ROUTE_BUILD_ENTRY = NOT_FOUND
BUILD = NOT_RUN
CORRECTNESS = NOT_RUN
LOCAL_RAW = NOT_RUN
EXTERNAL_PROCESS_OR_LEASE_STOP = NO (no device action in this check)
```

```text
TARGET_SECTION_LAYOUT = AVAILABLE (parent-only, exact artifact identity above)
TARGET_INSTRUCTION_DECODE = UNAVAILABLE (<not available> from explicit hiipu64 AICore decode)
INSTRUCTION_SIDE_PROFILE_OR_HOTNESS = NONE
SAME_PATH_BEFORE_AFTER_EFFECT = NOT_PROVEN
OFAT = NOT_PROPOSED
HYPOTHESIS_STATUS = FEASIBILITY_UNPROVEN / RESEARCH_ONLY
V001 = NOT_CREATED
```

NEXT_EXPECTED_STEP = Obtain a legal R3-owned build input/entry that is not a
Champion or other-route donor, or an authorized decoder/profile artifact that
provides actual HIIPU instructions or instruction-side hotness. Only then can
a single compiler/layout OFAT be proposed against a same-path generated-code
comparison. Until then, keep R3 research-only.

BLOCKER = INSTRUCTION_PAYLOAD_UNDECODABLE_AND_NO_ROUTE_BUILD_ENTRY: the existing
object yields real section/function sizes, but no instruction sequence or
hotness; the R3 worktree has no legal build entry for generating a comparison.
