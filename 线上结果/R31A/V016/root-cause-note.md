# 6ab3a527 root-cause note (read-only investigation)

Date: 2026-09-23
Constraint honored: no kernel/source edits, no Judge resubmit.

## Submissions

| submissionId | create_time (UTC) | status | kernel.asc sha256 | note |
|---|---|---|---|---|
| 6ab2c9790304f72a56a3c465 | 2026-09-22T18:31:21Z | Pass | dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0 | equals V016 worktree/online blob |
| 6ab35deb0304f72a56db63d6 | 2026-09-23T05:04:43Z | Compile Error | ab5bb3085f3800bbd4f99b633c4bb272cf5636f2739fe4a99bb17ef1e5cd8b1c | now resolved: judge-returned files[2].content |
| 6ab3a5270304f72a5607c3ef | 2026-09-23T10:08:39Z | Compile Error | 0e52d372b7e2d408e660888299e2c959d6d50c1525451df0870e15b699b9e739 | != dd130938; head starts at AscendC member decls (no includes) |

Q1: actual pre-submit SHA for 6ab3a527 == dd130938? **NO**.

## Problem metadata (current + historical)

- problemId: 6a9a9a99bf41025d6013eb85
- contest_id: 6a9a9295bf41025d601255a3
- code_template: npu_kernel_dev
- Same values recorded on 2026-09-11 in 归档 phase3 docs / problem API snapshot.

## Judge wrapper vs run_kernel ABI

- Judge main.asc calls run_kernel with 13 args ending (availableCoreNum, stream, epsilon).
- R31A-V016 and R31B-V011 both declare matching extern "C" void run_kernel(...13...).
- TensorGroupInfo layout matches judge predefinition on all three.
- Template side files (CMake/main/run.sh) identical sha across Pass and both CE submissions (main=edd8d27b…).

## Compiler diagnostics (from result[].msg)

- 6ab3a527: `kernel.asc:1:5: error: use of undeclared identifier 'AscendC'` (header/includes missing; starts mid-class members).
- 6ab35deb: `judge.asc:1097:2: error: expected '}'` + member undeclared — brace-structure damage in uploaded kernel.
- 6ab2c979: Pass 15/15.

## ROOT_CAUSE

**INPUT_DIFFERENT**

Not TEMPLATE_CHANGED / TOOLCHAIN_CHANGED / PROBLEM_CHANGED: wrapper ABI, problem ids, code_template, and template sibling files are stable; CE only when kernel.asc bytes differ from dd130938.
