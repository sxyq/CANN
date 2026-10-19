# 从 MIX-R014-R002-R019-V001 生成 R029-V001-WIDE-FP32-CACHED 主体
# 来源：ChatGPT 分享对话《编译并修复》节点 0958
# 原始脚本硬编码沙箱路径 /mnt/data/，本机复跑需替换为本地路径。

from pathlib import Path
import re, hashlib, json, zipfile, shutil, math, csv

src = Path("/mnt/data/MIX-R014-R002-R019-V001_kernel.asc")
text = src.read_text(encoding="utf-8")

# Version/header
text = text.replace(
"AddRmsNormBias - MIX-R014-R002-R019-V001",
"AddRmsNormBias - R029-V001-WIDE-FP32-CACHED",
1
)
text = text.replace(
" * Parent: MIX-R014-R002-V001\n * Status: candidate / not yet online tested\n *\n * ONLY new mechanism over the current Champion:\n *   - replace scalar rmsValue + vector Duplicate + vector Div\n *     with scalar invRms = 1/rms + vector Muls.\n *\n * Existing Champion mechanisms retained unchanged:\n *   - R014-V002: gamma/bias residency + low-precision FP32 parameter cache\n *   - R002: retain y=x+residual in UB for cached rows\n *\n * The R019 delta is applied to both the cached-row fast path and the\n * fallback D-style path. This remains one global mechanism change.\n",
""" * Parent: MIX-R014-R002-R019-V001
 * Status: candidate / not yet CANN-compiled or online tested here
 *
 * Single new mechanism:
 *   - add one R029 runtime mode for wide FP32 rows:
 *     when 4096 < D <= 32768 and average rows/core < 2, cache the complete
 *     y = x + residual row in UB and remove the second x/residual GM read.
 *
 * Existing mechanisms remain:
 *   - R014-V002 parameter residency for D <= 4096 with row reuse
 *   - R002 retained-y small-row path
 *   - R019 scalar reciprocal + vector Muls normalization
 *
 * R005/large-tile ideas are used only as a reference for the 7680-element
 * working tile needed to fit the wide cached row within UB. This is not an
 * R005-mainline rewrite.
""",
1
)

# Constants
text = text.replace(
"    static constexpr int32_t kScalarElems = 16;\n",
"""    static constexpr int32_t kScalarElems = 16;
    static constexpr int32_t kWideFp32CachedTileElems = 7680;
    static constexpr uint64_t kWideFp32CachedRowMaxWidth = 32768ULL;
""",
1
)

# Replace Init function entirely using regex from signature to before Process.
pattern = re.compile(
r"""    __aicore__ inline void Init\(
        GM_ADDR x, GM_ADDR residual, GM_ADDR gamma,
        GM_ADDR bias, GM_ADDR output\)
    \{
.*?
    \}

    __aicore__ inline void Process\(""",
re.S)
m = pattern.search(text)
if not m:
    raise RuntimeError("Init block not found")

new_init = r'''    __aicore__ inline void Init(
        GM_ADDR x, GM_ADDR residual, GM_ADDR gamma,
        GM_ADDR bias, GM_ADDR output,
        uint64_t rowWidth, uint64_t rowCount, uint32_t blockCount)
    {
        xGm_.SetGlobalBuffer((__gm__ T *)x);
        residualGm_.SetGlobalBuffer((__gm__ T *)residual);
        gammaGm_.SetGlobalBuffer((__gm__ T *)gamma);
        biasGm_.SetGlobalBuffer((__gm__ T *)bias);
        outputGm_.SetGlobalBuffer((__gm__ T *)output);

        wideFp32CachedMode_ = false;

        if constexpr (AscendC::IsSameType<T, float>::value) {
            const uint64_t averageRowsPerCore =
                blockCount == 0U
                    ? rowCount
                    : rowCount / static_cast<uint64_t>(blockCount);

            wideFp32CachedMode_ =
                rowWidth > kParamCacheMaxWidth &&
                rowWidth <= kWideFp32CachedRowMaxWidth &&
                averageRowsPerCore < 2ULL;
        }

        /*
         * R029-V001 mode:
         *
         * FP32 wide single/few-row path UB budget at D=32768:
         *
         *   full y row       32768 * 4 = 131072 B
         *   x tile           7680 * 4  = 30720 B
         *   residual tile    7680 * 4  = 30720 B
         *   scalar/partials  16 * 4    = 64 B
         *
         * total ~= 188.1 KiB.
         *
         * Do not allocate the generic FP32 work buffers in this mode.
         */
        if (wideFp32CachedMode_) {
            tpipe.InitBuffer(
                xBuf_,
                kWideFp32CachedTileElems * sizeof(float));

            tpipe.InitBuffer(
                residualBuf_,
                kWideFp32CachedTileElems * sizeof(float));

            tpipe.InitBuffer(
                yFp32Buf_,
                rowWidth * sizeof(float));

            tpipe.InitBuffer(
                sumBuf_,
                kScalarElems * sizeof(float));

            return;
        }

        /*
         * Existing MIX-R014-R002-R019 allocation remains unchanged.
         */
        tpipe.InitBuffer(xBuf_, kTileElems * sizeof(T));
        tpipe.InitBuffer(residualBuf_, kTileElems * sizeof(T));
        tpipe.InitBuffer(xFp32Buf_, kTileElems * sizeof(float));
        tpipe.InitBuffer(residualFp32Buf_, kTileElems * sizeof(float));
        tpipe.InitBuffer(yFp32Buf_, kTileElems * sizeof(float));
        tpipe.InitBuffer(sumBuf_, kScalarElems * sizeof(float));

        if constexpr (!AscendC::IsSameType<T, float>::value) {
            tpipe.InitBuffer(
                gammaFp32Buf_,
                kParamCacheOffset * sizeof(float));

            tpipe.InitBuffer(
                biasFp32Buf_,
                kParamCacheOffset * sizeof(float));
        }
    }

    __aicore__ inline void Process('''
text = text[:m.start()] + new_init + text[m.end():]

# Add wide tile length helper after TileLength.
needle = """    __aicore__ inline int32_t TileLength(uint64_t remain) const
    {
        return static_cast<int32_t>(
            remain > static_cast<uint64_t>(kTileElems) ? kTileElems : remain);
    }

"""
replacement = needle + """    __aicore__ inline int32_t WideFp32CachedTileLength(
        uint64_t remain) const
    {
        return static_cast<int32_t>(
            remain > static_cast<uint64_t>(kWideFp32CachedTileElems)
                ? kWideFp32CachedTileElems
                : remain);
    }

"""
if needle not in text:
    raise RuntimeError("TileLength block not found")
text = text.replace(needle, replacement, 1)

# Insert mode dispatch in Process after localRows.
needle = """        const uint64_t localRows = endRow > beginRow ? endRow - beginRow : 0ULL;
        const bool useParamCache = rowWidth <= kParamCacheMaxWidth && localRows > 1ULL;
"""
replacement = """        const uint64_t localRows = endRow > beginRow ? endRow - beginRow : 0ULL;

        /*
         * R029-V001: one new runtime mode only.
         */
        if (wideFp32CachedMode_) {
            for (uint64_t row = beginRow; row < endRow; ++row) {
                ProcessWideFp32CachedRow(
                    row,
                    rowWidth,
                    invRowWidth,
                    epsilon);
            }
            return;
        }

        const bool useParamCache =
            rowWidth <= kParamCacheMaxWidth &&
            localRows > 1ULL;
"""
if needle not in text:
    raise RuntimeError("Process dispatch point not found")
text = text.replace(needle, replacement, 1)

# Insert new path before PrepareParameterCache.
needle = "    __aicore__ inline void PrepareParameterCache(uint64_t rowWidth)\n"
if needle not in text:
    raise RuntimeError("PrepareParameterCache point not found")

wide_method = r'''    __aicore__ inline void ProcessWideFp32CachedRow(
        uint64_t row,
        uint64_t rowWidth,
        float invRowWidth,
        float epsilon)
    {
        /*
         * This function is reached only for T=float.
         *
         * The complete y row is retained in yFp32Buf_. xBuf_/residualBuf_
         * are streaming work tiles. The first pass computes partial sums while
         * retaining y. The output pass therefore reloads gamma/bias only.
         */
        const uint64_t rowOffset = row * rowWidth;

        AscendC::LocalTensor<float> xLocal =
            xBuf_.Get<float>();

        AscendC::LocalTensor<float> residualLocal =
            residualBuf_.Get<float>();

        AscendC::LocalTensor<float> valueLocal =
            yFp32Buf_.Get<float>();

        AscendC::LocalTensor<float> sumSlots =
            sumBuf_.Get<float>();

        int32_t tileIndex = 0;

        /*
         * Pass 1:
         *
         *   x + residual -> cached full row y
         *   square       -> residualLocal
         *   partial sum  -> sumSlots[tileIndex]
         */
        for (uint64_t col = 0;
             col < rowWidth;
             col += kWideFp32CachedTileElems) {

            const int32_t valid =
                WideFp32CachedTileLength(
                    rowWidth - col);

            Load(
                xLocal,
                xGm_,
                rowOffset + col,
                valid);

            Load(
                residualLocal,
                residualGm_,
                rowOffset + col,
                valid);

            SyncMTE2ToV();

            AscendC::Add(
                valueLocal[col],
                xLocal,
                residualLocal,
                valid);

            AscendC::PipeBarrier<PIPE_V>();

            /*
             * x/residual are dead after the Add.
             * residualLocal becomes square storage;
             * xLocal becomes ReduceSum workspace.
             */
            AscendC::Mul(
                residualLocal,
                valueLocal[col],
                valueLocal[col],
                valid);

            AscendC::PipeBarrier<PIPE_V>();

            AscendC::ReduceSum(
                sumSlots[tileIndex],
                residualLocal,
                xLocal,
                valid);

            AscendC::PipeBarrier<PIPE_V>();

            ++tileIndex;

            SyncVToMTE2();
        }

        /*
         * Reduce the <=5 tile partials and form invRms.
         */
        AscendC::ReduceSum(
            residualLocal,
            sumSlots,
            xLocal,
            tileIndex);

        AscendC::PipeBarrier<PIPE_V>();

        SyncVToS();

        const float squareSum =
            residualLocal.GetValue(0);

        const float meanSquare =
            squareSum * invRowWidth +
            epsilon;

        SyncSToV();

        AscendC::Duplicate(
            xLocal,
            meanSquare,
            1);

        AscendC::Sqrt(
            xLocal,
            xLocal,
            1);

        AscendC::PipeBarrier<PIPE_V>();

        SyncVToS();

        const float invRms =
            1.0f /
            xLocal.GetValue(0);

        SyncSToV();

        /*
         * xLocal/residualLocal were used by Vector and will now be overwritten
         * by gamma/bias MTE2 loads.
         */
        SyncVToMTE2();

        /*
         * Output pass:
         *
         * cached y
         *   -> Muls(invRms)
         *   -> Mul(gamma)
         *   -> Add(bias)
         *
         * No second x/residual GM load.
         */
        for (uint64_t col = 0;
             col < rowWidth;
             col += kWideFp32CachedTileElems) {

            const int32_t valid =
                WideFp32CachedTileLength(
                    rowWidth - col);

            Load(
                xLocal,
                gammaGm_,
                col,
                valid);

            Load(
                residualLocal,
                biasGm_,
                col,
                valid);

            SyncMTE2ToV();

            AscendC::Muls(
                valueLocal[col],
                valueLocal[col],
                invRms,
                valid);

            AscendC::PipeBarrier<PIPE_V>();

            AscendC::Mul(
                valueLocal[col],
                valueLocal[col],
                xLocal,
                valid);

            AscendC::PipeBarrier<PIPE_V>();

            AscendC::Add(
                valueLocal[col],
                valueLocal[col],
                residualLocal,
                valid);

            AscendC::PipeBarrier<PIPE_V>();

            /*
             * xLocal/residualLocal can be reused for the next parameter load.
             */
            SyncVToMTE2();

            SyncVToMTE3();

            Store(
                outputGm_,
                rowOffset + col,
                valueLocal[col],
                valid);

            SyncMTE3ToV();
        }
    }

'''
text = text.replace(needle, wide_method + needle, 1)

# Add bool member before tpipe.
needle = "private:\n    AscendC::TPipe tpipe;\n"
replacement = """private:
    bool wideFp32CachedMode_;
    AscendC::TPipe tpipe;
"""
if needle not in text:
    raise RuntimeError("private member point not found")
text = text.replace(needle, replacement, 1)

# Update kernel Init call.
needle = "    op.Init(x, residual, gamma, bias, output);\n"
replacement = """    op.Init(
        x,
        residual,
        gamma,
        bias,
        output,
        rowWidth,
        rowCount,
        blockCount);
"""
if needle not in text:
    raise RuntimeError("op.Init call not found")
text = text.replace(needle, replacement, 1)

# Write full candidate.
out = Path("/mnt/data/R029-V001-WIDE-FP32-CACHED_kernel.asc")
out.write_text(text, encoding="utf-8")

# Static checks.
brace = 0
brace_min = 0
for ch in text:
    if ch == "{":
        brace += 1
    elif ch == "}":
        brace -= 1
    brace_min = min(brace_min, brace)

static = {
    "version": "R029-V001-WIDE-FP32-CACHED",
    "parent": "MIX-R014-R002-R019-V001",
    "sha256": hashlib.sha256(out.read_bytes()).hexdigest(),
    "brace_final": brace,
    "brace_min": brace_min,
    "has_run_kernel": "extern \"C\" void run_kernel" in text,
    "has_device_kernel": "__global__ __vector__" in text,
    "new_mode": "ProcessWideFp32CachedRow" in text,
    "new_tile": 7680,
    "max_cached_width": 32768,
    "compile_status": "not tested in this environment"
}

Path("/mnt/data/R029-V001-WIDE-FP32-CACHED_STATIC.json").write_text(
    json.dumps(static, ensure_ascii=False, indent=2),
    encoding="utf-8"
)

# Record latest R011 result into a refreshed package and include next candidate.
source_pkg = Path("/mnt/data/AddRmsNormBias_ALL_IN_ONE_R011_CANDIDATE.zip")
pkg_work = Path("/mnt/data/_pkg_r011_result_r029")
if pkg_work.exists():
    shutil.rmtree(pkg_work)
pkg_work.mkdir()

with zipfile.ZipFile(source_pkg, "r") as z:
    z.extractall(pkg_work)

roots = [p for p in pkg_work.iterdir() if p.is_dir()]
pkg_root = roots[0] if len(roots) == 1 else pkg_work

(pkg_root/"code"/out.name).write_bytes(out.read_bytes())
(pkg_root/"code"/"R029-V001-WIDE-FP32-CACHED_STATIC.json").write_text(
    json.dumps(static, ensure_ascii=False, indent=2),
    encoding="utf-8"
)

# R011 result metrics under the previously agreed compact reporting convention.
baseline = [4.33,11.96,8.98,97.36,14.02,122.74,239.20,401.84,191.36,323.92,258.44,251.22,758.78,125000.0,11300.0]
r011 = [3.78,12.40,8.95,91.55,21.09,118.50,225.72,382.19,188.90,309.42,260.06,211.10,757.71,119000.0,14200.0]
best = [1.47,2.16,2.48,6.66,5.21,11.66,14.05,30.64,50.61,47.35,67.54,76.68,307.60,3750.0,8510.0]

baseline_improvement = (sum(baseline)-sum(r011))/sum(baseline)*100.0
score_recalc = sum(min(1.0,b/t) for b,t in zip(best,r011))/15*100.0

record = f"""# R011-V001 在线结果与下一版本

## R011-V001

- Correctness: 15/15 Pass
- 当前15点（μs，Case14/15已换算）:
  `3.78, 12.40, 8.95, 91.55, 21.09, 118.50, 225.72, 382.19, 188.90, 309.42, 260.06, 211.10, 757.71, 119000, 14200`
- 相对既定用户基线总耗时提升: `{baseline_improvement:.2f}%`
- 按当前 best/current 等权口径复算分数: `{score_recalc:.2f}`
- 决策: performance reject。手工树归约未形成整体收益，Case15 明显变慢。

## R005 / 参考实现审查

当前归档中没有一份可证明为“本地 V005 原始提交字节文件”的独立源码，因此不能声称已验证 V005 的完整 lineage。

可确认的是：
- R005 在统一路线中是“大 Tile”候选，不是全局主线；
- 已知高性能参考采用多种 Tile 和多条 dtype/shape 路径，不是单纯 R005；
- 下一版本只借用其 7680 wide-FP32 tile/UB 预算思想，不复制 R005 主线。

## 下一候选

`R029-V001-WIDE-FP32-CACHED`

Parent:
`MIX-R014-R002-R019-V001`

唯一新增:
- 为宽 FP32、少行/每核场景加入 full-row y cache；
- 4096 < D <= 32768；
- average rows/core < 2；
- tile = 7680；
- 删除该模式第二次 x/residual GM 读取。

其他小行缓存、参数驻留、R019 normalization、fallback 保持 Parent。
"""

docs_dir = pkg_root/"docs"
docs_dir.mkdir(exist_ok=True)
(docs_dir/"R011-V001结果与R029-V001计划.md").write_text(record, encoding="utf-8")

# Update current tree concise.
tree = """# 当前探索树

```mermaid
flowchart TD
    B["既定用户基线"]

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
    MIX --> MIX19["R014 + R002 + R019<br/>当前最佳实测组合"]
    R019 --> MIX19

    D --> R011["R011 Manual Reduction"]
    R011 --> X11["15/15<br/>Performance Reject"]

    MIX19 --> R029["R029-V001<br/>Wide FP32 Cached Row<br/>下一候选"]

    R005["R005 Large Tile<br/>仅作为 UB/Tile 参考"] -. "7680 tile/UB思想" .-> R029
```
"""
(docs_dir/"当前探索树.md").write_text(tree, encoding="utf-8")

# Recompute checksums and zip.
sha_path = pkg_root/"SHA256SUMS.txt"
hash_lines = []
for p in sorted(pkg_root.rglob("*")):
    if p.is_file() and p != sha_path:
        hash_lines.append(
            f"{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.relative_to(pkg_root)}"
        )
sha_path.write_text("\n".join(hash_lines)+"\n", encoding="utf-8")

pkg_out = Path("/mnt/data/AddRmsNormBias_ALL_IN_ONE_R011_RESULT_R029_NEXT.zip")
if pkg_out.exists():
    pkg_out.unlink()
with zipfile.ZipFile(pkg_out, "w", zipfile.ZIP_DEFLATED) as z:
    for p in sorted(pkg_root.rglob("*")):
        if p.is_file():
            z.write(
                p,
                Path("AddRmsNormBias_ALL_IN_ONE_R011_RESULT_R029_NEXT") / p.relative_to(pkg_root)
            )

with zipfile.ZipFile(pkg_out, "r") as z:
    bad = z.testzip()

print(f"R011 baseline improvement: {baseline_improvement:.2f}%")
print(f"R011 score recalc: {score_recalc:.2f}")
print(f"R029 source: {out}")
print(f"R029 SHA256: {static['sha256']}")
print(f"Static braces: final={brace}, min={brace_min}")
print(f"Package: {pkg_out}")
print(f"ZIP integrity: {'PASS' if bad is None else bad}")
