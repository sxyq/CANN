# CANNJudge 提交自动化专项调查（只读调查 + 结果轮询工具）

> 调查日期：2026-09-17
> 专项范围：评估并实现"浏览器登录态提交 + 自动查询提交结果"，减少手工操作。
> 硬约束：线上提交属外部操作，只有在用户明确输入"确认提交 &lt;版本路径&gt;"后才允许真实 POST。
> 本文件记录的所有接口结论都来自**匿名 GET / HEAD / OPTIONS** 实测，未执行任何 POST/PUT/PATCH/DELETE，未读取或保存任何 Cookie、密码、Token、授权头，未访问浏览器配置数据库，未修改 Kernel 和服务器文件。

## 0. 三种证据状态（后文全部按此标注）

| 标注 | 含义 |
| --- | --- |
| **【已实测】** | 本次调查真实发出请求并拿到结果，或本地真实执行成功 |
| **【源码推断】** | 只从公开前端 JS 读出来的逻辑，未在真机验证 |
| **【无法验证】** | 当前环境不具备验证条件，明确说明限制，不写成已实现 |

---

## 1. 接口调查结论

### 1.1 提交接口

| 项目 | 结论 | 证据 |
| --- | --- | --- |
| 路径 | `POST https://cannjudge.cn/api/submissions/submit` | 【源码推断】`js/pages/problem/editor.js:847` |
| 请求体 | `{ problemId, files:[{path,content}], userId, tiling_h, tiling_key_h, host_cpp, kernel_cpp }` | 【源码推断】`editor.js:707-714` + `editor.js:843-850` |
| `files` 口径 | 只含**可编辑**文件；`npu_kernel_dev` 模板下允许 `*.asc` / `*.h`，受保护文件为 `judge.asc`、`data_utils.h`、`main.asc` | 【源码推断】`editor.js:182-187` |
| legacy 四字段 | 由 `projectFilesToLegacyPayload()` 从工程文件推导；对只有一个 `kernel.asc` 的直调工程，`tiling_h / tiling_key_h / host_cpp / kernel_cpp` 会**全部是空串** | 【源码推断】`editor.js:138-177` |
| 响应取值 | 前端只读 `result.data.submissionId` | 【源码推断】`editor.js:855-858` |
| 方法支持 | `OPTIONS /api/submissions/submit` → 200；`HEAD` → 500 | 【已实测】 |
| **鉴权口径** | **未验证**。按约束未发 POST，因此无法判断"仅凭 `userId` 不带 Cookie 能否提交"。旁证：用户维度的读取接口返回 `401 请先登录`，说明服务端确实校验登录态 | 【无法验证】 |

### 1.2 结果查询接口（本专项的关键发现）

`GET https://cannjudge.cn/api/submissions/{submissionId}` —— **无需登录即可读取**。

| 验证动作 | 结果 |
| --- | --- |
| 不存在的 id（匿名） | `404 {"message":"submission not found"}` —— 注意是 **404 而不是 401**，说明该路由没有登录门 |
| 真实 id（匿名） | `200`，完整返回 15 个点位结果 |【已实测】|

返回结构（`GET /api/submissions/{id}`）：

```text
_id, ID, user_id, problem_id, create_time, status, valid, theory_score, result[],
problem{_id,name,title,contest_id}, contest, group, user{_id,ID,nickname},
can_view_code, files[], tiling_h, tiling_key_h, tiling_key_cpp, host_cpp, kernel_cpp
```

`result[]` 每项：`testcase_id, time, precision_ratio, testcase_status, type, best_time, msg`

三条必须记住的口径：

1. **`time` 单位是微秒（μs）**，不是毫秒。前端 `testcaseTimeLabel()` 按 `≥1000 → 转 ms` 显示。
2. **`precision_ratio` 是"匹配率"**，页面显示的误差是 `(1 - ratio) × 100%`。参考样本 15 点全为 `1` → 失配率 0.0000%。
3. **这个接口没有总分字段**，`result[].score` 恒为 `0`（未填充）。官方总分只能从排行榜接口取。

### 1.3 题目解析接口

`GET /api/problems/name/{token}` —— 匿名可读。【已实测】

| 字段 | 实测值 |
| --- | --- |
| problemId | `6a9a9a99bf41025d6013eb85`（与 AGENTS.md 记录一致） |
| 平台题目编号 | `1742` |
| title | `AddRmsNormBias` |
| contest_id | `6a9a9295bf41025d601255a3` |
| code_template | `npu_kernel_dev`（确认直调模板） |
| 测试点 | 15 个，公开 `tbest` 分别为 1.47 / 2.16 / 2.48 / 6.66 / 5.21 / 11.66 / 14.05 / 30.64 / 50.61 / 47.35 / 67.54 / 76.68 / 307.6 / 3750.12 / 8506.03 μs，`baseline` 全为 `null` |

### 1.4 排行榜接口

`GET /api/problems/{problemId}/ranking?page=&size=` —— 匿名可读，支持 `page` / `size`。【已实测】

- `total = 353`，`pages = 18`（`size=20`）或 `4`（`size=100`）；`rows[]` 每项含
  `rank / user_id / user / team / submission_id / status / create_time / score / result[]`，顶层另有 `testcases[]`（15 项，含 `tbest`）。
- **只收录 `Pass` 状态的提交**：第一页 100 条 `status` 全部为 `Pass`。
- `score` 就是官方总分（样本 `74.41`）。

### 1.5 分数公式已复现（重要副产品）

用公开样本 `6aaa74eeb0477ec41e147510`（rank 1，官方 `score = 74.41`）的 15 点 `time` 与排行 `testcases[].tbest` 代入平台公式
`s_i = 100 / (1 + log_1.5(t_i / T_i))`，取均值得到 **74.411**，与官方值一致；且该提交 `result[].best_time` 与排行 `testcases[].tbest` **15 项逐项相等**。

结论：
1. 平台公式 `100/(1+log₁.₅(t/T))` 的口径正确；
2. `tbest` 就是公式里的 `T_i`；
3. 因此**只用一个 `GET /api/submissions/{id}` 就能完整复算总分**，不需要额外接口。

按项目约定，复算值只作诊断，性能收敛仍只认排行榜里的官方总分。

### 1.6 一个需要登录的接口（对照组）

`GET /api/submissions/user/{uid}/problem/{pid}` → `401 {"message":"请先登录"}`。【已实测】

也就是说：**"看某个用户的全部提交"需要登录态，"看单条提交结果"和"看公开排行"不需要。** 本专项因此可以把自动化的重心完全放在只读侧。

---

## 2. 浏览器自动化是否可行

### A. CUA / Computer Use —— 不可用【已实测】

`computer-use`、`control-chrome`、`control-in-app-browser` 三个 skill 都要求先做同一件事：

```js
const { setupComputerUseRuntime } = await import("<plugin root>/scripts/computer-use-client.mjs");
```

它们全部依赖 **Node REPL 工具 `mcp__node_repl__js`**（`control-chrome` / `control-in-app-browser` 则是 `browser-client.mjs` + `agent.browsers.*`）。
本会话做工具检索时**没有找到 `node_repl` 类工具**（返回的是无关工具），这些插件根目录也没有对应的 `*-client.mjs`。

结论：**三个 skill 在本环境无法 bootstrap，CUA 通道不成立。** 这是环境缺失，不是权限拒绝。

### B. Playwright / CDP —— 通道在，但没有登录态【已实测】

| 检查项 | 实测结果 |
| --- | --- |
| `agent-browser` CLI | **未安装**（`command not found`）；插件自带 `scripts/setup.sh` 需联网安装，约 500MB Chromium |
| `playwright-core` | **可用**，版本 `1.62.1`，位于 `~/.workbuddy/binaries/node/workspace/node_modules` |
| 浏览器二进制 | 已缓存 `chromium-1234`，`executablePath()` 指向 `Google Chrome for Testing.app` |
| 本机已登录的 Chrome | **不存在**：`/Applications/Google Chrome.app` 未安装（只有 Edge） |
| CDP 调试端口 | `9222 / 9223 / 9333` 全部**无监听**；Chrome 进程为空 |
| `node` 可用性 | `node -e "console.log('ok')"` → `ok`（满足 agent-browser 的前置要求） |

结论：**可以编程驱动一个浏览器，但只能驱动 Playwright/agent-browser 自带的独立 profile。**
它拿不到用户现有浏览器里的登录态，位置也不在任何用户日常浏览器里。
要用它提交，就必须让用户在**这个受控浏览器窗口里重新登录一次**——这是一项需要用户明确接受的前置成本，不是"自动获得登录态"。

### C. 登录页面内执行同源 fetch —— 技术上可行，本会话无法执行【源码推断 + 无法验证】

站点响应头（【已实测】）：

```text
content-security-policy: default-src 'self'; script-src 'self' 'nonce-…';
  connect-src 'self'; object-src 'none'; frame-ancestors 'none'
```

- `connect-src 'self'` ⇒ 在 `cannjudge.cn` 页面里对 `/api/...` 发 fetch 是同源请求，**会自动带上会话 Cookie**，这正是本题设想的通道。
- `script-src 'self' 'nonce-…'` ⇒ **外部注入脚本会被 CSP 拦掉**（拿不到 nonce）。
- 前端 `api()` 的实现是 `fetch(url, {method, headers:{Content-Type}})`，**没有显式写 `credentials`**，即使用的浏览器默认 `same-origin`——所以会话确实走 Cookie，而不是 `Authorization` 头。【源码推断】

因此：这条路只能由**用户在已登录页面的控制台里手打一段 fetch** 来完成，Agent 既不能注入脚本（无 nonce、无 node_repl），也不允许读取 Cookie 去复刻会话。
**本次未执行任何 POST，所以"页面内 fetch 能否成功提交"仍属未验证，不得写成已实现。**

### D. 独立 CLI 只做结果轮询 —— 可行，已实现【已实测】

因为 `GET /api/submissions/{id}` 和 `GET /api/problems/...` 都是匿名可读，一个纯 CLI 就能完整承担"查结果"这一半。

---

## 3. 结果轮询是否可行

**可行，且已完成实现与实测。** 轮询工具：`调研/工具/cannjudge.py`（全仓库仅此一份）。

| 能力 | 实现方式 | 实测状态 |
| --- | --- | --- |
| 输入 submissionId | 位置参数 | 已实测 |
| 结果查询 | 只发 `GET /api/submissions/{id}` | 已实测（真实 id 返 200、假 id 返 404 并 exit 2） |
| 轮询间隔可配置 | `--interval`（默认 15s）、`--max-wait`（默认 1800s）、`--once` | 已实测 |
| 终态判定 | 复刻前端 `statusKey()`：pass/skipped/wrong/fail 为终态，waiting 为非终态；并按 `PASS/ACCEPTED/SKIP/WRONG/FAIL/COMPILE/RUNTIME/...` 关键词收敛 | 已实测（对 `Pass` 样本一次即停）|
| 终态输出 | 总状态 + `N/15` 通过点位 + 逐点状态 / 失配率 / 耗时 / 基准 / msg | 已实测，输出见 §3.1 |
| 总分 | 本地按平台公式复算；`--official` 额外读公开排行取官方总分 | 已实测（复算 74.411 / 官方 74.41）|
| 不保存登录信息 | 代码里只发匿名 GET，不含 Cookie/Authorization；用 `ProxyHandler({})` 默认直连 | 已自检 |
| 不执行提交 | 文件内**不存在**任何 POST/PUT/PATCH/DELETE 代码路径 | 已自检 |
| 附带能力 | `preflight <版本路径>` 本地只读输出路径/入口名/行数/字节数/SHA-256/模板体检；`problem` 输出 problemId 与 15 点公开 tbest | 已实测 |

### 3.1 实测输出（公开样条，只读）

```text
=== CANNJudge 提交前核对（本地只读，未联网）===
版本目录      : …/提交/混合方案/H001-正确性优先/V005
本机文件名    : kernel.txt        平台入口名 : kernel.asc
行数          : 394               字节数     : 16690
SHA-256       : afd5b8eef071e2e849064b1913290b485e0689cf9da40987fb1ffc67dc36bd9c
```

```text
提交 ID        : 6aaa74eeb0477ec41e147510   (平台 ID 325510)
总状态         : Pass   [判定 pass]   有效提交
通过点位       : 15 / 15
官方总分       : 74.41    ← 来自公开排行榜，可作收敛依据
总分(本地复算) : 74.411   ← 按平台公开公式复算，仅作诊断
 #  状态            失配率%     耗时us     基准us   单点分(复算)
 1  Pass            0.0000      2.14      1.47       51.92
11  Pass            0.0000    159.82     67.54       32.01
15  Pass            0.0000   9071.01   8506.03       86.31
```

（上例用的是公开排行榜第 1 名提交的公开数据，仅用于验证工具链路。）

---

## 4. 推荐的最小实现方式

```text
① 本地 preflight 锁定文件身份
   python3 调研/工具/cannjudge.py preflight 提交/混合方案/H00N-名称/V00N/kernel.txt
   → 把行数 / 字节数 / SHA-256 写进该版本的 结果.md

② 用户在【自己的浏览器】里完成提交

③ 从跳转后的页面 URL 取到 submissionId，交给 CLI

④ python3 调研/工具/cannjudge.py poll <submissionId> --interval 15 --official
   → 终态后输出总状态 + 逐点结果 + 官方总分

⑤ 把 poll 的输出写回 提交/混合方案/H00N-名称/V00N/结果.md 与 提交/版本实验记录.md
```

**为什么不把"自动提交"一起做出来：** 提交必须有登录态，而本环境里唯一可编程驱动的浏览器是 Playwright 自带的独立 profile，用户还得在里面重新登录一次；`agent-browser` 未安装；CUA 通道缺失。在这个前提下做出来的"提交助手"只能算半自动，却要额外承担"token 落到磁盘""重复 POST""把编译通过写成线上通过"三类风险，收益不划算。

如果将来确实要做提交助手，必须同时满足下面全部条件才允许动手：

1. 用户在受控浏览器内**自己**完成一次登录，凭据不经过 Agent、不落盘；
2. 提交前必须回显版本路径 / 文件名 / 行数 / 字节数 / SHA-256，并**等待运行时明确确认**；
3. 每个候选**只允许发一次 POST**，代码层面禁止循环重试；
4. 两次 POST 之间强制 ≥120 秒；
5. 拿到 submissionId 后自动转 `poll` 轮询；
6. 失败版本**不得自动重投**；
7. 严禁把"本地编译通过"写成"线上通过"。

---

## 5. 需要用户手动确认的步骤

1. **提交动作本身**：在浏览器里打开题目 submit 页，**全选替换** `kernel.asc` 内容（不能只贴片段），点"提交代码"。
2. **提供 submissionId**：从跳转后的 URL 里复制，交给 Agent。
3. **人工核对上传完整性**：平台对非本人源码只返回占位内容，**无法远程反查上传内容**。所以必须以 §4① 的 SHA-256 / 行数 / 字节数为准做人工比对。
4. **确认是否要启用自动提交**：需要用户先接受"在受控浏览器里重新登录一次"这一前提。
5. 每日额度（用户提供的 50 次/天、取最后一次成绩）仍无公开页面依据，由用户掌握。

---

## 6. 未解决问题与风险

| # | 问题 | 状态 / 缓解 |
| --- | --- | --- |
| 1 | POST 的真实鉴权口径（`userId` 能否替代 Cookie） | **未验证**；按约束不测。旁证是用户维度读接口 401 |
| 2 | 无法远程反查上传内容 | `can_view_code=false` 时 `files` 只返回 23 字节占位 `permission.txt`。缓解：preflight 锁哈希 + 人工核对 |
| 3 | 隐藏测试点 | 若平台把某些点标为 `Hidden`，逐点误差不可见（当前样本 15/15 全可见）；`Hidden` 点不计入通过数 |
| 4 | 官方总分只在排行榜里，且排行榜**只收录 Pass** | 非 Pass 提交没有官方分值，与历史口径"无有效分数"一致，不是数据缺失 |
| 5 | 结果接口无总分字段 | 已缓解：`--official` 读排行；或按已复现的公式本地复算（标注为复算值）|
| 6 | 本机出网偶发被拦截 | 工具默认直连（避免沙箱 `HTTP_PROXY` 劫持）；失败时 `--proxy http://127.0.0.1:7897` |
| 7 | 自动提交通道 | 不具备稳定可控的已登录浏览器，**不做**；只有 §4 全部条件满足才做 |

---

## 7. 顺带发现的与项目约定不一致处（只报告，未修改任何文件）

1. **`提交/混合方案/H001-正确性优先/V005/kernel.txt` 首尾行与 checklist 不符**
   实际首行 `#include <cstdint>`、末行 `#endif`；`文档/submission-checklist.md` §6.1 第④项写的是"首行 `#include <cmath>`、末行 `}`"。
   该项是 V002 时代的核对方式，V005 现在是 `#if/#endif` 包裹的形态。`preflight` 会把这两条标为"需确认"，需要人工判断是否更新 checklist 口径。
2. **`V005/kernel.txt` 与 `V005/kernel.txt.pre-repair` 字节完全相同**
   两者 SHA-256 均为 `afd5b8ee…`（16690 字节 / 394 行）。`pre-repair` 目前是重复副本。**本次未删除、未改动**，仅登记事实。

---

## 8. 本次调查的边界声明

- 未执行任何线上提交，未发出任何 POST / PUT / PATCH / DELETE。
- 未读取、未输出、未保存任何 Cookie / 密码 / Token / 授权头。
- 未访问浏览器配置数据库、密码库或 profile。
- 未修改 Kernel、源码、服务器文件；新增文件仅 `调研/工具/cannjudge.py` 与本文件。
- 所有"可行"结论都附有实测命令或源码位置；所有"未验证"项都明确标注，未写成已实现。
