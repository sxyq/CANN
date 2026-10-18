# Canonical Route Runner 工具归档

本目录保存 `cann-tooling-runner` 工作树中缺失于整理后 `main` 的 Runner 源码、测试 fixture 和历史失败记录。它们是工具开发材料，不是 Kernel Candidate，也没有被当作 Correctness 或 Local 后端结果。

## 状态

- 来源分支：`tooling/canonical-route-runner`，HEAD `3b7b2cb1028744ff51f5f5e31321879f53f02d7f`。
- 当前工作树中的四个文件还有未提交修改；归档内容按当前工作树字节保存，必须以文件 SHA 和本说明为准，不表示已形成正式版本提交。
- 现有记录显示 Runner 构建失败：ACLRT wrapper 报 `Unknown kernelInfo` / `undefined symbol: __origin__tooling_fixture_kernel`，未生成可用 `route_runner` 或候选共享库；没有 Correctness、Canonical Local 或 Official 结果。
- 本目录不包含 `build-smoke/`、`build-abi-smoke/`、共享库、对象文件或其他 CMake 生成产物。

## 内容

- `route-runner/`：CMake、构建脚本、ABI 头文件、Parent/Candidate 适配源码、Runner host 源码。
- `fixtures/`：`abi_smoke_fixture.asc` 与 `fp32_rms_norm_bias_fixture.asc` 两个测试 fixture。
- `W4-ACLRT-LAUNCH-ATTEMPT-20261007.{log,md}`：ACLRT 启动失败记录。
- `W4-PLUGIN-REGISTRATION-DIAGNOSTIC.md`：插件注册诊断，记录失败边界；没有对应的原始 stdout/stderr。

以上内容仅作为可追溯的工具源码和失败证据保存，不改变项目的 Kernel 成绩、评分公式或线上提交状态。
