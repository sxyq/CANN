# Agent01 比赛题面 / 规则 / 提交接口

> 2026-09-11 只读调研。未改 `源码/提交/文档`，未上传 CANNJudge。
> 状态：1=官方页面明确写出；2=用户提供但无页面依据；3=页面可访问但内容不完整；4=接口/源码推断；5=无法确认。

## 结论摘要

1. 题面、15 测试点、得分公式、直调模板、上传字段均可从 CANNJudge 公开 API 直接取得（1）。
2. 公式 `100/(1+log₁.₅(t/T))`、T=该点全局最优、总分=15 点均值，已用榜一数据反算对齐（1+4）。
3. 本题 `npu_kernel_dev`：可编辑仅 `kernel.asc`；上传 `POST /api/submissions/submit` 的 `files`，不是旧 msopgen 四字段（1）。
4. 西南目前只有初赛；GitCode 报名页可开但为 SPA，正文抓不到（3）。
5. 每日约 50 次、判题 15 点 shape/dtype/eps、SoC 型号均未公开（2/5）。

## 已确认事项

| 项 | 结论 | 状态 |
| --- | --- | --- |
| 赛事 | 2026年CANN挑战赛_西南赛区（初赛）；contest_id=`6a9a9295bf41025d601255a3`；zone_id=`2094722165106008066` | 1 |
| 题目 | AddRmsNormBias / `addrmsnormbias` / 平台 ID 1742 / problem_id=`6a9a9a99bf41025d6013eb85` | 1 |
| 时间 | 2026-09-05 00:00 ~ 2026-10-17 18:00（CST；API 存 UTC） | 1 |
| 报名 | 必填，外链 `https://competition.gitcode.com/competition/2094722165106008066/intro`；`organization_mode=gitcode_team` | 1 |
| 排行 | `ranking_mode=score`，`ranking_submission_mode=latest`，同分比提交时间早 | 1 |
| 封榜 | `freeze_ranking=true`，结束前 30 分钟 | 1 |
| 测试点 | **15** 个 default；全部通过才计分；每点 `iterations=5` | 1 |
| 计分 | 单点 `100/(1+log₁.₅(t/T))`，总分均值；榜一反算 80.6867≈80.69 | 1+4 |
| 精度 | fp32 相对/绝对 <1e-4；fp16/bf16 <1e-3；int32 完全准确（模板遗留） | 1 |
| 语义 | `y=x+r; rms=sqrt(mean(y²)+eps); out=y/rms*gamma+bias` | 1 |
| IO | x/residual 同 `(...,D)`；gamma/bias=`(D,)`；out 同 x；ND | 1 |
| dtype/rank | fp16/bf16/fp32 输出同型；2D/3D/4D | 1 |
| 范围 | batch[1,8192]，seq[1,32768]，D[64,32768]；D 可非 32 倍数 | 1 |
| epsilon | float，默认 1e-5，题面称通常 1e-5~1e-6 | 1 |
| 特殊值 | NaN/Inf 不崩溃；结果确定性 | 1 |
| 环境 | CANN 9.0.0；`code_template=npu_kernel_dev`；`kernel_pattern=vector`；`use_baseline=false` | 1 |
| 上传 | 仅 `kernel.asc` 可编辑；保护 `judge.asc/data_utils.h/main.asc`；自定义根文件限 .asc/.h 最多 20 个 | 1 |
| 入口 | `extern "C" void run_kernel(..., availableCoreNum, stream, float epsilon)`；`__global__ __vector__` | 1 |
| dtype 枚举 | 0=fp32 1=fp16 2=bf16 | 1 |
| 默认 SoC | 模板 CMake 默认 `dav-2201`，可 `-DNPU_ARCH` 覆盖 | 4 |
| 本地样例 | FP16 `[1,64]`，eps=1e-5 | 1 |
| 本地核验 | `isclose(rtol=0.001,atol=0.001)`+失配 0.1%（判题是否同口径未证） | 1/5 |
| golden | 全 FP32 计算，末尾一次 cast | 1 |
| 榜上耗时 | 单点约 1.5μs~9.0ms；通过点 `precision_ratio=1` | 1 |
| 规模 | 354 队尝试 / 12757 次提交 / 213 队通过 / 240 队在榜 | 1 |
| 其它元数据 | `score_mode=1`，`scoring_rule=default`，`show_total_ranking=true`，`huawei_contest_type=ict` | 1 |
| 开源仓 PR | 本题 `submit_enabled=false`，不走 GitCode PR 判题 | 1 |
| 赛区说明 | 面向四川/重庆/云南/贵州/西藏高校在校学生 | 1 |

## 未确认事项

| 项 | 状态 | 说明 |
| --- | --- | --- |
| 每日约 50 次 | 2 | 题面/提交页/前端 JS 无明文；榜上多队累计 300+（多成员），不能证伪 |
| 15 点 shape/dtype/eps | 5 | ranking 只暴露 id/tbest |
| 判题精度算法 | 5 | 本地 0.1% 失配容忍是否上判题端未知 |
| SoC 型号 | 4/5 | 仅模板默认 dav-2201 |
| 西南决赛/违规细则 | 5 | 无独立页面；江山赛区有 prelim+final，仅模式旁证 |
| GitCode 正文 | 3 | HTTP 200，Next.js 无服务端正文 |
| T 实时更新 | 4 | ranking.`tbest` 随提交变化，支持动态 T |

### 排行可见的 15 点 TBest（μs，状态 1）

`1.47, 2.16, 2.54, 6.80, 5.66, 12.23, 16.70, 31.36, 50.61, 47.59, 132.31, 76.68, 411.34, 3750.12, 8656.25`

后两点数量级远高于前段，是性能主战场；但 15 点 shape/dtype 仍不可见，不能反推测试配置。

### 提交接口要点（状态 1）

- `POST /api/submissions/submit`，JSON：`problemId`、`userId`、`files=[{path,content}]`。
- 本题 `files` 实际只需 `{path:"kernel.asc"}`；后端会带上 legacy 四字段兼容，可忽略。
- 模板包可 `GET /api/problems/{id}/package` 下载，内含 `kernel.asc/main.asc/CMakeLists.txt/run.sh/scripts/*`。
- 未登录可读题面/模板/排行；提交与编辑器草稿需登录。

## 来源清单

| # | 对象 | 等级 | 状态 |
| --- | --- | --- | --- |
| S1 | `cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85` 题面 JSON | A | 1 |
| S2 | `.../api/contests/name/op_challenge_xinan_prelim?groupId=69c8ffcea2bcbfa3591f481e` | A | 1 |
| S3 | `.../api/problems/6a9a9a99bf41025d6013eb85/package` 官方模板 zip（8 文件） | A | 1 |
| S4 | `.../api/problems/6a9a9a99bf41025d6013eb85/ranking` 15 点 tbest/得分 | A | 1 |
| S5 | `.../api/problems/.../template` 可编辑文件树 | A | 1 |
| S6 | `cannjudge.cn/js/pages/problem/editor.js` 提交表单与保护文件 | A | 1 |
| S7 | `.../api/submissions/contest/6a9a9295bf41025d601255a3/stats` | A | 1 |
| S8 | `competition.gitcode.com/competition/2094722165106008066/intro` | C | 3 |
| S9 | `.../api/contests/group/69c8ffcea2bcbfa3591f481e` 决赛模式旁证 | A | 1 |
| S10 | `hiascend.com/developer/contests/details/0bdb8bf112724e44b7ffe7787b05a1bd` | B | 3 |

访问日均为 2026-09-11。A=官网 API/模板；B=官方活动页；C=第三方赛事 SPA。

## 对实现路线的约束

1. 提交物收敛到 `kernel.asc` 单文件；入口签名与 `__global__ __vector__` 逐字对齐模板。
2. 性能是唯一计分维度；15 点全过才进均值。榜上第 14/15 点约 3.8/9.0ms，拉分最大，优先攻大 D。
3. T 动态上移：先保证过线正确，再冲性能；同分比提交时间，稳定后尽早提交。
4. D 非 32 倍数、大 outer、fp16 溢出必须自测；本地模板只有 FP16 `[1,64]`，不能当判题全集。
5. 日额度与 SoC 未公开：提交次数保守记账；真机默认 `dav-2201`，并准备 `-DNPU_ARCH` 覆盖。
6. GitCode 报名是参赛门槛，与判题解耦；本题不走开源仓 PR。
