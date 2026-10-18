# asc-devkit Agent 配置

`.agent` 维护 `asc-devkit` 仓内的本地 skill 和 skill 级测试。`.opencode`、`.claude`、`.codex` 是兼容入口目录，目录内入口软链指向 `.agent`，不是独立事实源。

本目录面向两类读者：

- 人类使用者：查看 skill 能力和运行测试。
- AI Agent：按仓内事实源维护 skill、引用 reference、生成或补齐 API UT。

## 给人类使用者

### 当前内置 skill

| Skill | 用途 | 入口 |
|------|------|------|
| `asc-api-ut-gen` | 生成或补齐 `tests/api/` 下的 AscendC API UT | `skills/asc-api-ut-gen/SKILL.md` |
| `asc-ar-dev` | AscendC 需求分析、实现方案、host/device 代码评审与验证规划 | `skills/asc-ar-dev/SKILL.md` |
| `asc-npu-arch` | 维护芯片、架构、`SocVersion`、`__NPU_ARCH__` 和 dtype 事实 | `skills/asc-npu-arch/SKILL.md` |
| `public-skills` | 从 `https://gitcode.com/cann-agent/skills.git` 安装或更新公共远程 skill | `skills/public-skills/SKILL.md` |
| `gitcode-pr` | 通过公共远程 skill 处理 GitCode PR、评论、讨论和变更查询 | `skills/gitcode-pr/SKILL.md` |

当前 `.agent` 通过 `asc-ar-dev` 覆盖 AscendC 需求分析、实现方案和代码评审；示例工程生成暂不单独维护 skill。

### 目录入口

```text
.agent/                 # 唯一事实源
.agent/skills/          # skill 源目录
.agent/tests/           # skill 级测试
.opencode/
.opencode/README.md -> ../.agent/README.md
.opencode/skills    -> ../.agent/skills
.opencode/tests     -> ../.agent/tests
.claude/
.claude/README.md   -> ../.agent/README.md
.claude/skills      -> ../.agent/skills
.claude/tests       -> ../.agent/tests
.codex/
.codex/README.md    -> ../.agent/README.md
.codex/skills       -> ../.agent/skills
.codex/tests        -> ../.agent/tests
```

以后新增或修改 skill，只改 `.agent/skills`。

### 默认使用

仓内本地维护 `asc-api-ut-gen`、`asc-ar-dev`、`asc-npu-arch` 和 `public-skills`。`gitcode-pr` 等公共通用 skill
使用 `skills/public-skills/scripts/install-public-skills.sh` 从 `cann-agent/skills` 安装到
`.agent/skills/_remote/`，并通过 `skills/gitcode-pr -> _remote/gitcode-pr` 这类一级软链暴露给客户端。

更新公共通用 skill：

```bash
bash .agent/skills/public-skills/scripts/install-public-skills.sh
```

客户端直接读取对应兼容入口：

```text
.opencode/skills -> ../.agent/skills
.claude/skills   -> ../.agent/skills
.codex/skills    -> ../.agent/skills
```

以后新增或修改 skill，只改 `.agent/skills`。`.opencode`、`.claude`、`.codex` 不应维护独立内容。

### 用户目录安装

日常使用不需要安装；本仓不再维护用户目录安装脚本。如果某个客户端只读取 `${CODEX_HOME:-$HOME/.codex}/skills`，用户目录安装由客户端或用户环境自行管理，不作为本仓事实源的一部分。

兼容入口损坏时，恢复为“真实目录 + 内部软链”，不要把入口根目录恢复成指向 `.agent` 的软链：

```text
mkdir -p .opencode .claude .codex
ln -s ../.agent/README.md .opencode/README.md
ln -s ../.agent/skills .opencode/skills
ln -s ../.agent/tests .opencode/tests
ln -s ../.agent/README.md .claude/README.md
ln -s ../.agent/skills .claude/skills
ln -s ../.agent/tests .claude/tests
ln -s ../.agent/README.md .codex/README.md
ln -s ../.agent/skills .codex/skills
ln -s ../.agent/tests .codex/tests
```

### 测试

运行全部 skill 测试：

```bash
bash .agent/tests/skills/run-all.sh
```

单独测试：

```bash
bash .agent/tests/skills/asc-api-ut-gen/test.sh
bash .agent/tests/skills/asc-ar-dev/test.sh
bash .agent/tests/skills/asc-npu-arch/test.sh
```

## 给 AI Agent

### 维护边界

- `.agent/skills` 是本仓 skill 的唯一事实来源。
- 公共通用 skill 的本仓事实源是 `skills/public-skills` 的安装脚本；不要在本仓重新实现
  `gitcode-pr`、`gitcode-issue`、`api-doc-generator` 或 `gitcode-pipeline`。
- `skills/_remote/` 是远程缓存目录，`skills/gitcode-pr` 等一级入口应是指向 `_remote/<skill>` 的软链。
- 不直接编辑 `.opencode/skills`、`.claude/skills` 或 `.codex/skills`；它们都通过软链访问 `.agent/skills`。
- `.opencode`、`.claude`、`.codex` 的根路径应保持为真实目录；根目录软链可能触发沙箱只读保留路径校验失败。
- 当前 `asc-api-ut-gen` skill 忽略 Tensor API，不维护 `include/experimental/tensor_api/`、`impl/experimental/tensor_api/`、`tests/api/tensor_api/` 的生成规则。

### 事实源顺序

| 事实类型 | 首选来源 |
|---------|----------|
| 芯片、架构、`SocVersion`、`__NPU_ARCH__`、dtype | `skills/asc-npu-arch/references/npu-arch-facts.json` |
| 架构和 dtype 人工说明 | `skills/asc-npu-arch/references/npu-arch-guide.md` |
| AscendC 需求分析、实现评审、调用层级和环境解析 | `skills/asc-ar-dev/SKILL.md` 和 `skills/asc-ar-dev/references/` |
| API 类型、目录和 UT 目录映射 | `skills/asc-api-ut-gen/references/foundations/api-directory-map.md` |
| API 类型限制和生成器约束 | `skills/asc-api-ut-gen/references/foundations/generation-constraints.json` |
| UT 生成流程、覆盖率和报告要求 | `skills/asc-api-ut-gen/references/workflows/` |

脚本需要消费芯片或 dtype 事实时，读取结构化 JSON，不解析 Markdown 表格，也不要在脚本中复制完整事实表。

### 修改规则

- 修改 skill 正文、reference、脚本或测试后，运行所有相关的 shell 测试脚本，并至少包含相关 skill 的 `test.sh`；确认执行正确后再交付。
- 跨 skill 或通用事实变更后，运行 `bash .agent/tests/skills/run-all.sh`，确认执行正确后再交付。
- 新增 reference 时，同步更新对应 `references/README.md` 或 skill 的参考索引。
- 新增芯片时，优先更新 `asc-npu-arch` 的结构化事实；只有 API 类型限制变化时，才更新 `asc-api-ut-gen` 的生成约束。

### UT 任务入口

处理 API UT 任务时：

1. 先确认目标 API、API 类型、目标芯片和是否需要构建验证。
2. 读取 `asc-npu-arch` 确认芯片、架构和 dtype 边界。
3. 读取 `asc-api-ut-gen` 的 API guide、目录映射和工作流。
4. 生成或修改 UT 后，报告 token 消耗、总耗时、当前覆盖率；无法获取时说明原因。
5. 不声称未执行的构建、UT 或覆盖率验证已经通过。
