# Four-Section Output Format

This file defines the default response structure for this skill.

Source of the structure: the bundled AscendC four-section response contract.

Important: the source file contributes only the structure, not the debug-bus topic. Use this format for AscendC requirement-document generation and requirement-code implementation unless the user explicitly asks for another format.

## Required Heading Order

```md
## 需求
## 背景简述
## 涉及领域
## 方案详述
```

## What Each Section Should Contain

### 需求

- The concrete thing to build, change, review, or debug.
- The expected output shape: code, patch, design, root-cause analysis, or verification plan.
- If the input bundle is missing, ask for `DEVKIT_PATH`, `CANN_PATH`, `SOC_ARCH`, and `TASK_GOAL` before producing a devkit-backed plan. If `TASK_GOAL` already contains the first three values, use them directly.

### 背景简述

- Relevant user-provided codebase path, resolved toolkit path, target SoC, current implementation shape, or known constraints.
- Existing behavior only. Avoid solution details here.

### 涉及领域

- The AscendC areas involved in the task.
- Typical values: basic API, high-level API, API dtype support, Host runtime, vector pipeline, matmul or tiling, memory movement, debug or dump, compiler metadata, compile engineering, CMake, scripts, compatibility.

### 方案详述

- The actual design or implementation.
- Include the chosen template, function split, launch flow, buffer movement, debug method, and verification strategy.
- When CANN, devkit, or SoC details matter, include `DEVKIT_PATH`, `CANN_PATH`, and `SOC_ARCH` explicitly. If `TASK_GOAL` already contains those values, use them directly.

## Minimal Skeleton

```md
## 需求
说明目标与交付物。

## 背景简述
说明现有代码、环境、SoC、依赖与限制。

## 涉及领域
列出 Host/Device、tiling、debug、DumpTensor、编译链等相关域。

## 方案详述
给出实现方案、代码结构、关键接口、验证方式与必要假设；如涉及本地环境，写明 `DEVKIT_PATH`、`CANN_PATH` 和 `SOC_ARCH`。
```

## Usage Rule

If the user asks directly for code, keep the final user-facing answer concise, but still derive the internal reasoning from this structure.
