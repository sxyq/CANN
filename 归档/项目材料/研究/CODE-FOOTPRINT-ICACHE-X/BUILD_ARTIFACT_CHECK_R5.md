# R3 Historical Build and Target Artifact Check

AGENT_ID = W5-R03 CODE-FOOTPRINT-ICACHE-X / Parfit
ROUTE = CODE-FOOTPRINT-ICACHE-X
BRANCH = research/w5-r03-code-footprint
WORKTREE = /home/data4t2/lelinfeng/cann-w5-r03-icache
CHECKED_AT = 2026-10-10T08:45:27+00:00
CURRENT_REVISION = NONE (V001 not created)

## Scope

This check stayed inside the R3 worktree. No other route source, Champion
source, shared tooling, Agent, Worktree, Runner, Candidate, or compiler system
was read, copied, or modified. The prior target-disassembly failure path was
not repeated.

## Commands and device observation

Command:

```text
npu-smi info
```

Host: `hwnput3`. `npu-smi` reported eight healthy Ascend 910B3 devices. HBM
usage and calculated free HBM were:

```text
device 0: 25362 / 65536 MB, free 40174 MB
device 1: 25418 / 65536 MB, free 40118 MB
device 2: 63767 / 65536 MB, free 1769 MB
device 3: 17614 / 65536 MB, free 47922 MB
device 4: 59214 / 65536 MB, free 6322 MB
device 5: 59737 / 65536 MB, free 5799 MB
device 6: 59735 / 65536 MB, free 5801 MB
device 7: 24790 / 65536 MB, free 40746 MB
```

Devices 0, 1, 3, and 7 satisfy the `FREE_HBM > 100 MB` build threshold.
Existing external processes were only observed and not stopped: rayWorkerDict,
VLLMWorker, python3.11, VLLMEngineCor, VLLMWorker_TP, and python3.

## Existing build-entry check

The following route-owned paths were checked for an existing historical build
entry:

```text
./研究/CODE-FOOTPRINT-ICACHE-X
./本地实验/CODE-FOOTPRINT-ICACHE-X
```

Result:

```text
./研究/CODE-FOOTPRINT-ICACHE-X: receipts only; no CMakeLists.txt, Makefile,
  build script, compile script, or target input
./本地实验/CODE-FOOTPRINT-ICACHE-X: absent
```

The absolute authorized compiler paths are present, but there is no legal
route-owned historical build entry or non-Champion input source to invoke from
this worktree. A build was therefore not started; creating a replacement
build system or borrowing another route/Champion source would violate scope.

```text
NEW_SOURCE_SHA = NONE (no build input)
NEW_ARTIFACT_SHA = NONE (no build output)
TEXT_SECTION_LAYOUT = NONE (no new target artifact)
BUILD = NOT_STARTED / ROUTE_BUILD_ENTRY_MISSING
```

## Gate

```text
NEW_TARGET_ARTIFACT = NONE
TARGET_INSTRUCTION_EVIDENCE = NONE
SAME_PATH_BEFORE_AFTER = NOT_PROVEN
GENERATED_CODE_CHANGE_PROVEN = NO
OFAT = NOT_PROPOSED
FEASIBILITY_STATUS = FEASIBILITY_UNPROVEN / RESEARCH_ONLY
V001 = NOT_CREATED
```

The HBM condition is available, but it cannot establish target code evidence
without a permitted build input and existing build entry. Source byte/line
counts are not used as I-cache evidence.

## Next step and blocker

NEXT_EXPECTED_STEP = Provide or authorize a route-scoped historical build
entry/input that is not another route or Champion donor; then run one real
build on an eligible device and record source/artifact SHA plus target text
section/layout. If no such entry is supplied, keep this route frozen.

BLOCKER = ROUTE_BUILD_ENTRY_MISSING / FEASIBILITY_UNPROVEN: HBM is available,
but the current worktree has no permitted build input or historical build
entry, so no new decodable target artifact can be produced.
