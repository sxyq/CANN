# 由 V003 + V002 合并生成 R029-V004-WIDE-LOWP-CACHED
# 来源：ChatGPT 分享对话《编译并修复》节点 1059
# 原始脚本硬编码沙箱路径 /mnt/data/，本机复跑需替换为本地路径。

from pathlib import Path
import re, hashlib, json, zipfile, shutil, csv, math, textwrap, os

v2_path = Path("/mnt/data/R029-V002-WIDE-FP16-CACHED_kernel.asc")
v3_path = Path("/mnt/data/R029-V003-WIDE-BF16-CACHED_kernel.asc")
v2 = v2_path.read_text(encoding="utf-8")
v3 = v3_path.read_text(encoding="utf-8")

# Start from V003 and add the already validated FP16 branch from V002.
text = v3

# Header
header_pat = re.compile(r"/\*\n \* AddRmsNormBias - R029-V003-WIDE-BF16-CACHED.*?\*/", re.S)
header = """/*
 * AddRmsNormBias - R029-V004-WIDE-LOWP-CACHED
 *
 * Parent:
 *   MIX-R014-R002-R019-V001
 *
 * Combination status:
 *   R029-V002 FP16 wide cached-y: independently online validated 15/15.
 *   R029-V003 BF16 wide cached-y: independently online validated 15/15,
 *   official platform score 28.68.
 *
 * Single combination objective:
 *   Combine the two already validated, dtype-disjoint low-precision wide-row
 *   cached-y paths in one submission.
 *
 * FP16 path:
 *   - 8192 < D <= 32768
 *   - rowCount <= blockCount
 *   - compute RMS from FP32 y
 *   - retain y in FP16 UB cache
 *   - skip the second x/residual GM read
 *
 * BF16 path:
 *   - 8192 < D <= 32768
 *   - rowCount <= blockCount
 *   - compute RMS from FP32 y
 *   - retain y in FP16 UB cache
 *   - skip the second x/residual GM read
 *
 * Existing parent mechanisms remain unchanged outside those two branches:
 *   - R014-V002 parameter residency for small rows
 *   - R002 retained-y small-row path
 *   - R019 scalar reciprocal + vector Muls normalization
 *
 * R029-V001 FP32 wide-cache is NOT included in this version because it has
 * not yet been independently validated online.
 *
 * Candidate status:
 *   Static checks only in this environment; no local CANN compile claimed.
 */"""
text = header_pat.sub(header, text, count=1)

# Constants: add FP16 constants next to BF16 constants.
text = text.replace(
"""    static constexpr int32_t kWideBf16CachedTileElems = 7680;
    static constexpr uint64_t kWideBf16CachedRowMaxWidth = 32768ULL;
""",
"""    static constexpr int32_t kWideFp16CachedTileElems = 7680;
    static constexpr uint64_t kWideFp16CachedRowMaxWidth = 32768ULL;
    static constexpr int32_t kWideBf16CachedTileElems = 7680;
    static constexpr uint64_t kWideBf16CachedRowMaxWidth = 32768ULL;
""", 1)

# Init: add bool init and FP16 condition.
text = text.replace(
"""        wideBf16CachedMode_ = false;

        if constexpr (AscendC::IsSameType<T, bfloat16_t>::value) {
""",
"""        wideFp16CachedMode_ = false;
        wideBf16CachedMode_ = false;

        if constexpr (AscendC::IsSameType<T, half>::value) {
            wideFp16CachedMode_ =
                rowWidth > static_cast<uint64_t>(kTileElems) &&
                rowWidth <= kWideFp16CachedRowMaxWidth &&
                rowCount <= static_cast<uint64_t>(blockCount);
        }

        if constexpr (AscendC::IsSameType<T, bfloat16_t>::value) {
""", 1)

# Replace V003-specific allocation if with combined low-precision allocation.
old_alloc_pat = re.compile(
r"""        /\*
         \* R029-V003 BF16-only wide cached-row mode\.
.*?
        if \(wideBf16CachedMode_\) \{
.*?
            return;
        \}
""", re.S)
m = old_alloc_pat.search(text)
if not m:
    raise RuntimeError("V003 allocation block not found")
combined_alloc = """        /*
         * R029-V004 shared low-precision wide cached-row UB layout.
         *
         * Both FP16 and BF16 use 16-bit x/residual/cache storage plus three
         * 7680-element FP32 work tiles. At D=32768 the main payload is about
         * 184 KiB, so the generic small-row FP32 parameter caches are not
         * allocated in this mode.
         */
        if (wideFp16CachedMode_ || wideBf16CachedMode_) {
            const int32_t wideTileElems =
                wideFp16CachedMode_
                    ? kWideFp16CachedTileElems
                    : kWideBf16CachedTileElems;

            tpipe.InitBuffer(
                xBuf_,
                wideTileElems * sizeof(T));

            tpipe.InitBuffer(
                residualBuf_,
                wideTileElems * sizeof(T));

            tpipe.InitBuffer(
                outputBuf_,
                rowWidth * sizeof(T));

            tpipe.InitBuffer(
                valueFp32Buf_,
                wideTileElems * sizeof(float));

            tpipe.InitBuffer(
                xFp32Buf_,
                wideTileElems * sizeof(float));

            tpipe.InitBuffer(
                residualFp32Buf_,
                wideTileElems * sizeof(float));

            tpipe.InitBuffer(
                sumBuf_,
                kScalarElems * sizeof(float));

            return;
        }
"""
text = text[:m.start()] + combined_alloc + text[m.end():]

# Process dispatch: insert FP16 dispatch before BF16 dispatch.
dispatch_needle = """        if constexpr (AscendC::IsSameType<T, bfloat16_t>::value) {
            if (wideBf16CachedMode_) {
"""
dispatch_repl = """        if constexpr (AscendC::IsSameType<T, half>::value) {
            if (wideFp16CachedMode_) {
                for (uint64_t row = beginRow; row < endRow; ++row) {
                    ProcessWideFp16CachedRow(
                        row,
                        rowWidth,
                        invRowWidth,
                        epsilon);
                }
                return;
            }
        }

        if constexpr (AscendC::IsSameType<T, bfloat16_t>::value) {
            if (wideBf16CachedMode_) {
"""
if dispatch_needle not in text:
    raise RuntimeError("BF16 dispatch not found")
text = text.replace(dispatch_needle, dispatch_repl, 1)

# Add FP16 TileLength helper before BF16 helper.
bf16_helper = """    __aicore__ inline int32_t WideBf16CachedTileLength(
        uint64_t remain) const
"""
fp16_helper = """    __aicore__ inline int32_t WideFp16CachedTileLength(
        uint64_t remain) const
    {
        return static_cast<int32_t>(
            remain > static_cast<uint64_t>(kWideFp16CachedTileElems)
                ? kWideFp16CachedTileElems
                : remain);
    }

"""
if bf16_helper not in text:
    raise RuntimeError("BF16 helper point not found")
text = text.replace(bf16_helper, fp16_helper + bf16_helper, 1)

# Extract entire FP16 method from V002.
start_token = "    __aicore__ inline void ProcessWideFp16CachedRow("
start = v2.index(start_token)
end_token = "    __aicore__ inline void PrepareParameterCache("
end = v2.index(end_token, start)
fp16_method = v2[start:end]

# Insert FP16 method before BF16 method in V004.
bf16_method_token = "    __aicore__ inline void ProcessWideBf16CachedRow("
idx = text.index(bf16_method_token)
text = text[:idx] + fp16_method + text[idx:]

# Add FP16 bool member.
text = text.replace(
"""private:

    bool wideBf16CachedMode_;
""",
"""private:

    bool wideFp16CachedMode_;
    bool wideBf16CachedMode_;
""", 1)

# Save
v4_path = Path("/mnt/data/R029-V004-WIDE-LOWP-CACHED_kernel.asc")
v4_path.write_text(text, encoding="utf-8")

# Static checks
brace = 0
brace_min = 0
for ch in text:
    if ch == "{":
        brace += 1
    elif ch == "}":
        brace -= 1
    brace_min = min(brace_min, brace)

static = {
    "version": "R029-V004-WIDE-LOWP-CACHED",
    "parent": "MIX-R014-R002-R019-V001",
    "combined_validated_routes": [
        "R029-V002-WIDE-FP16-CACHED",
        "R029-V003-WIDE-BF16-CACHED"
    ],
    "excludes": ["R029-V001-WIDE-FP32-CACHED (not yet online validated)"],
    "sha256": hashlib.sha256(v4_path.read_bytes()).hexdigest(),
    "brace_final": brace,
    "brace_min": brace_min,
    "has_run_kernel": 'extern "C" void run_kernel' in text,
    "has_device_kernel": "__global__ __vector__" in text,
    "fp16_method_count": text.count("ProcessWideFp16CachedRow"),
    "bf16_method_count": text.count("ProcessWideBf16CachedRow"),
    "compile_status": "not tested in this environment",
    "online_status": "not tested"
}
static_path = Path("/mnt/data/R029-V004-WIDE-LOWP-CACHED_STATIC.json")
static_path.write_text(json.dumps(static, ensure_ascii=False, indent=2), encoding="utf-8")

# Metrics for V003
baseline = [
    4.33,11.96,8.98,97.36,14.02,122.74,239.20,401.84,191.36,
    323.92,258.44,251.22,758.78,125000.0,11300.0
]
v3times = [
    3.95,8.96,7.20,32.86,14.89,50.58,80.66,201.65,110.12,
    181.36,183.76,126.50,611.77,49400.0,10800.0
]
baseline_total = sum(baseline)
v3_total = sum(v3times)
baseline_improvement = (baseline_total-v3_total)/baseline_total*100.0
official_score_v3 = 28.68

# Extract latest package and update docs/results.
base_pkg = Path("/mnt/data/AddRmsNormBias_ALL_IN_ONE_R029_V002_RESULT_V003_NEXT.zip")
work = Path("/mnt/data/_pkg_r029_v003_result_v004_next")
if work.exists():
    shutil.rmtree(work)
work.mkdir()
with zipfile.ZipFile(base_pkg, "r") as z:
    z.extractall(work)
roots = [p for p in work.iterdir() if p.is_dir()]
root = roots[0] if len(roots)==1 else work

# Copy candidate
(root/"code"/v4_path.name).write_bytes(v4_path.read_bytes())
(root/"code"/static_path.name).write_bytes(static_path.read_bytes())

# Score methodology doc correction
score_doc = f"""# 分数与性能口径（更新至 R029-V003）

## A. 基线提升百分比

“基线提升”只比较用户最开始指定的基线版本与当前版本的 **15 个测试点总耗时**。

先统一单位：
- μs 保持不变；
- ms × 1000 转为 μs。

公式：

`BaselineImprovement = (ΣT_baseline - ΣT_current) / ΣT_baseline × 100%`

固定用户基线总耗时：

`ΣT_baseline = {baseline_total:.2f} μs`

R029-V003：

`ΣT_current = {v3_total:.2f} μs`

所以：

`BaselineImprovement = {baseline_improvement:.2f}%`

这项指标用于我们自己的性能进展跟踪，也与 2026 CANN 算子天梯赛公开规则中“所有测试用例耗时总和提升比例”的描述一致。

## B. 官方平台分数

**官方分数不再从每个测试点的“最优用时”列自行反推。**

原因：
此前使用的本地近似公式 `100/N × Σ(best_i/current_i)` 与平台实际 Score 不一致。

从现在开始：

`OfficialScore = 平台提交历史表中的“分数/Score”字段`

R029-V003 平台实际：

`OfficialScore = 28.68`

如果某轮用户只提供 15 个 Case 耗时、没有提供平台 Score：
- 文档写 `OfficialScore = 未提供/待记录`；
- 不再伪造或用本地近似值替代。

## C. 每轮固定汇报字段

1. Correctness / Pass 状态
2. 相对固定用户基线的总耗时提升百分比
3. 官方平台 Score（只读平台字段）
4. 下一版本完整代码
5. 当前探索树
6. 归档包
"""
(root/"docs"/"分数与性能口径_更新至R029-V003.md").write_text(score_doc, encoding="utf-8")

# V003 result record
record = f"""# R029-V003 在线结果与 R029-V004 下一实验

## R029-V003-WIDE-BF16-CACHED

平台记录：
- Status: Pass
- Official Score: **{official_score_v3:.2f}**
- Submit time: `2026/09/17 15:13:49`

15 Case:
`3.95, 8.96, 7.20, 32.86, 14.89, 50.58, 80.66, 201.65, 110.12, 181.36, 183.76, 126.50, 611.77, 49.4ms, 10.8ms`

统一换算后总耗时：
`{v3_total:.2f} μs`

相对固定用户基线：
`{baseline_improvement:.2f}%`

## 结论

- 15/15 Pass。
- BF16 wide cached-y 路径通过线上 correctness。
- 平台真实 Score 为 28.68。
- 不再使用 best/current 本地公式冒充官方 Score。

## 下一版本：R029-V004-WIDE-LOWP-CACHED

目的：
把已经分别在线验证通过的两个 **dtype-disjoint** 路线合并：
- R029-V002: FP16 wide cached-y
- R029-V003: BF16 wide cached-y

不包含尚未独立在线验证的 R029-V001 FP32 wide-cache。

这样可以验证 V002 与 V003 是否能在一个提交中同时保留收益，同时仍保持因果清晰。
"""
(root/"docs"/"R029-V003结果与R029-V004计划.md").write_text(record, encoding="utf-8")

# Update exploration tree
tree = f"""# 当前探索树

```mermaid
flowchart TD
    BASE["固定用户基线"]

    D["D baseline"]

    D --> R015["R015 Multi-row DMA"]
    R015 --> X15["冻结"]

    D --> R028["R028 Scalar Sync"]
    R028 --> X28["Reject"]

    D --> R002["R002 Retained-y"]
    D --> R014["R014 Parameter Residency"]
    R002 --> MIX["R014 + R002"]
    R014 --> MIX

    D --> R013["R013 Double Buffer"]
    R013 --> X13["Reject"]

    D --> R019["R019 Reciprocal + Muls"]
    MIX --> MIX19["R014 + R002 + R019"]

    D --> R011["R011 Manual Reduction"]
    R011 --> X11["Performance Reject"]

    MIX19 --> V1["R029-V001<br/>Wide FP32 Cached<br/>待独立验证"]

    MIX19 --> V2["R029-V002<br/>Wide FP16 Cached<br/>15/15 Pass"]

    MIX19 --> V3["R029-V003<br/>Wide BF16 Cached<br/>15/15 Pass<br/>Score 28.68<br/>基线提升 {baseline_improvement:.2f}%"]

    V2 --> V4["R029-V004<br/>FP16 + BF16 Wide Cached<br/>下一验证"]
    V3 --> V4

    R005["R005 Large Tile<br/>仅作 Tile / UB 参考"]
    R005 -. "7680 Tile / UB" .-> V1
    R005 -. "7680 Tile / UB" .-> V2
    R005 -. "7680 Tile / UB" .-> V3
```
"""
(root/"docs"/"当前探索树.md").write_text(tree, encoding="utf-8")

# Update summary.json
summary_path = root/"results"/"summary.json"
if summary_path.exists():
    summary = json.loads(summary_path.read_text(encoding="utf-8"))
else:
    summary = {}
summary["R029-V003-WIDE-BF16-CACHED"] = {
    "status": "Pass",
    "official_score": 28.68,
    "submit_time": "2026/09/17 15:13:49",
    "per_case_us": v3times,
    "total_us": v3_total,
    "baseline_total_us": baseline_total,
    "baseline_improvement_pct": baseline_improvement,
    "score_source": "platform displayed Score field"
}
summary["next_candidate"] = {
    "version": "R029-V004-WIDE-LOWP-CACHED",
    "purpose": "combine independently validated FP16 and BF16 wide cached-y modes",
    "source_sha256": static["sha256"],
    "status": "candidate"
}
summary["score_method"] = {
    "baseline_improvement": "(sum(baseline)-sum(current))/sum(baseline)*100",
    "official_score": "read directly from platform Score field; do not infer from per-case best/current",
    "local_best_current_formula": "deprecated as official score"
}
summary_path.write_text(json.dumps(summary, ensure_ascii=False, indent=2), encoding="utf-8")

# Update RESULTS.md
results_md = root/"results"/"RESULTS.md"
res_text = results_md.read_text(encoding="utf-8") if results_md.exists() else ""
res_text += f"""

## R029-V003-WIDE-BF16-CACHED
- Status: Pass
- Official Score: **28.68**
- Submit time: 2026/09/17 15:13:49
- Baseline total improvement: **{baseline_improvement:.2f}%**
- Per-case: 3.95, 8.96, 7.20, 32.86, 14.89, 50.58, 80.66, 201.65, 110.12, 181.36, 183.76, 126.50, 611.77 μs, 49.4 ms, 10.8 ms

Scoring rule in this archive:
- baseline improvement: total-latency formula;
- official score: platform Score field only.
"""
results_md.write_text(res_text, encoding="utf-8")

# Update current state
state_path = root/"handoff"/"CURRENT_STATE.json"
state = json.loads(state_path.read_text(encoding="utf-8")) if state_path.exists() else {}
state["latest_online"] = {
    "version": "R029-V003-WIDE-BF16-CACHED",
    "status": "Pass",
    "official_score": 28.68,
    "submit_time": "2026/09/17 15:13:49",
    "baseline_improvement_pct": baseline_improvement,
    "per_case_us": v3times
}
state["next_candidate"] = {
    "version": "R029-V004-WIDE-LOWP-CACHED",
    "source_sha256": static["sha256"],
    "status": "candidate",
    "includes": ["R029-V002 FP16 wide cache", "R029-V003 BF16 wide cache"],
    "excludes": ["R029-V001 FP32 wide cache"]
}
state["scoring"] = {
    "baseline_improvement_formula": "(sum(baseline)-sum(current))/sum(baseline)*100",
    "official_score_policy": "use platform displayed Score field only",
    "deprecated_local_score_formula": "100/N * sum(best/current) is diagnostic only, not official"
}
state_path.write_text(json.dumps(state, ensure_ascii=False, indent=2), encoding="utf-8")

# Add note to START_HERE
start_path = root/"START_HERE.md"
start_text = start_path.read_text(encoding="utf-8")
start_text += f"""

## 最新状态：R029-V003
- Pass
- Official Score: **28.68**
- 固定基线总耗时提升: **{baseline_improvement:.2f}%**
- 下一候选: `R029-V004-WIDE-LOWP-CACHED`
- 分数口径见: `docs/分数与性能口径_更新至R029-V003.md`
"""
start_path.write_text(start_text, encoding="utf-8")

# Regenerate SHA and zip
sha_path = root/"SHA256SUMS.txt"
hash_lines = []
for p in sorted(root.rglob("*")):
    if p.is_file() and p != sha_path:
        hash_lines.append(f"{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.relative_to(root)}")
sha_path.write_text("\n".join(hash_lines)+"\n", encoding="utf-8")

pkg_path = Path("/mnt/data/AddRmsNormBias_ALL_IN_ONE_R029_V003_RESULT_V004_NEXT.zip")
if pkg_path.exists():
    pkg_path.unlink()
with zipfile.ZipFile(pkg_path, "w", zipfile.ZIP_DEFLATED) as z:
    for p in sorted(root.rglob("*")):
        if p.is_file():
            z.write(p, Path("AddRmsNormBias_ALL_IN_ONE_R029_V003_RESULT_V004_NEXT")/p.relative_to(root))

with zipfile.ZipFile(pkg_path) as z:
    bad = z.testzip()
    file_count = len(z.namelist())

print("V4 source", v4_path)
print("V4 SHA", static["sha256"])
print("brace", brace, "min", brace_min)
print("FP16 method refs", static["fp16_method_count"], "BF16 method refs", static["bf16_method_count"])
print("V3 total us", v3_total)
print("baseline improvement", baseline_improvement)
print("package", pkg_path, "files", file_count, "integrity", bad)
