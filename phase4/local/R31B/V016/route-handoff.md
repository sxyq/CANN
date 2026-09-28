# R31B Route Handoff

日期：2026-09-25

## 接续范围

- Worktree：`/Users/sunyiyang/Desktop/Project/cann-sixlane/R31B`
- Branch：`exec/sixlane-20260923-r31b`
- 当前版本：V016；直接父版本：R31B-V011；V011 官方分数：45.16。
- V016 仍为 `NEEDS_ONE_MORE_LOCAL`。只保留 V016，不创建 V017。
- Candidate 与 Parent kernel 源本轮均未改动。

## 已完成

- 旧编译输出指出 runner 曾从全局作用域引用未限定名称。当前 `paired_runner.cpp` 的函数类型和调用点均明确限定 `r31b_v011::` / `r31b_v016::`。
- 2026-09-25 21:27（上海时间）经 `cann-server3` 在 `hwnput3` 对 `paired_runner_v016` 强制重新编译并链接，结果 PASS。
- 旧失败记录仍在 `support/paired/build-v016-attempt1.log`；本次成功记录在 `support/paired/paired-build-v016-success.txt`，远端详细构建输出也已保留。
- paired launcher 已填入新 ELF 的 SHA-256，并具备 `--same-binary` 和 `--paired` 两种模式；P/C 模式要求四个确切形状均有通过记录，且输入源与 runner SHA-256 相符。
- 未启动 runner；未运行 correctness；未做时序采样。

## SHA-256 身份

| 对象 | SHA-256 |
|---|---|
| V011 Parent kernel source | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` |
| V016 Candidate kernel source | `9f5c353e65a13a740fe97dc7e6415df032d27560831a3ad142c77592b8208eb5` |
| paired wrapper source | `b827960aa2d60e6efd953b30f3ecfcfab17af5111f4d03e5309c4c94c68d1a55` |
| paired runner C++ | `d8a574735bda3e07cbfbc1de3c890a72fb7b437983eae38ea22d0bf8b0bd03bc` |
| paired ABI header | `096c0229f5d9e6fa37460808387517280cb0f2e4296e07b8c37f75cd73c89617` |
| paired CMake input | `29eb6e937ca94fbf2877ae22d2ee9a864066f8e5844ea0ccdc356c1ea40a3f3a` |
| remote runner ELF | `6e9337a2c1bf4e9c74db56f55d90fd5eb233fdb7f6f4d189265b0a8f80cdd7fc` |

远端 ELF：`/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/paired-runner-src/build/paired_runner_v016`，581456 bytes，AArch64 PIE，可执行。parent、Candidate、wrapper、C++、ABI、CMake 的本地与远端 SHA-256 均匹配。

## 当前限制

Main review 决定：R31B V016 保持 `NEEDS_ONE_MORE_LOCAL`。V011/V016 exact-source runner 的编译和链接均已通过；没有执行 runner，也没有采集测量数据。不要创建 V017。

当前 server3 没有可用设备：d0-d3 有活动负载，d4-d6 由 Main-2 租用，时序流程排除 d7。保留现有构建、失败和历史探测记录；本 Route 继续停留在 V016，等待获准的设备窗口。

后续只有在 Main 授权的设备窗口开放后，才继续逐形状运行同一可执行文件资格测试。四个形状全部 PASS 且 SHA-256 相符之前，不得开始 P/C 配对。当前状态下不运行 runner、correctness 或性能采样。
