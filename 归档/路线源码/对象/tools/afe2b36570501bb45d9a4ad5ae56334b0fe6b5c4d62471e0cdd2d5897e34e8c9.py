# 从 MIX-R014-R002-R019-V001 生成 R029-V002-WIDE-FP16-CACHED
# 来源：ChatGPT 分享对话《编译并修复》节点 0986
# 原始脚本硬编码沙箱路径 /mnt/data/，本机复跑需替换为本地路径。

from pathlib import Path
import re, hashlib, json, zipfile, shutil

src = Path("/mnt/data/MIX-R014-R002-R019-V001_kernel.asc")
text = src.read_text(encoding="utf-8")

# Header / version
text = text.replace(
    "AddRmsNormBias - MIX-R014-R002-R019-V001",
    "AddRmsNormBias - R029-V002-WIDE-FP16-CACHED",
    1,
)
text = text.replace(
""" * Parent: MIX-R014-R002-V001
 * Status: candidate / not yet online tested
 *
 * ONLY new mechanism over the current Champion:
 *   - replace scalar rmsValue + vector Duplicate + vector Div
 *     with scalar invRms = 1/rms + vector Muls.
 *
 * Existing Champion mechanisms retained unchanged:
 *   - R014-V002: gamma/bias residency + low-precision FP32 parameter cache
 *   - R002: retain y=x+residual in UB for cached rows
 *
 * The R019 delta is applied to both the cached-row fast path and the
 * fallback D-style path. This remains one global mechanism change.
""",
""" * Parent: MIX-R014-R002-R019-V001
 * Status: candidate / not yet CANN-compiled or online tested here
 *
 * Single new mechanism:
 *   - add one FP16-only wide-row cached-y runtime mode.
 *   - when 8192 < D <= 32768 and rowCount <= blockCount, retain the complete
 *     y = x + residual row in a compact FP16 cache and remove the second
 *     x/residual GM read for that row.
 *
 * Existing mechanisms retained:
 *   - R014-V002 parameter residency for small rows
 *   - R002 retained-y small-row path
 *   - R019 scalar reciprocal + vector Muls normalization
 *
 * This is an independent sibling of R029-V001 (FP32 wide cached row);
 * it does not depend on R029-V001 being accepted.
""",
1)

# Constants
text = text.replace(
    "    static constexpr int32_t kScalarElems = 16;\n",
"""    static constexpr int32_t kScalarElems = 16;
    static constexpr int32_t kWideFp16CachedTileElems = 7680;
    static constexpr uint64_t kWideFp16CachedRowMaxWidth = 32768ULL;
""",
1)

# Replace Init with shape-aware allocation
pat = re.compile(
r"""    __aicore__ inline void Init\(
        GM_ADDR x, GM_ADDR residual, GM_ADDR gamma,
        GM_ADDR bias, GM_ADDR output\)
    \{
.*?
    \}

    __aicore__ inline void Process\(""", re.S)
m = pat.search(text)
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

        wideFp16CachedMode_ = false;

        if constexpr (std::is_same<T, half>::value) {
            wideFp16CachedMode_ =
                rowWidth > static_cast<uint64_t>(kTileElems) &&
                rowWidth <= kWideFp16CachedRowMaxWidth &&
                rowCount <= static_cast<uint64_t>(blockCount);
        }

        /*
         * R029-V002 FP16-only wide cached-row mode.
         *
         * At D=32768 the main UB payload is approximately:
         *
         *   x half tile          7680 * 2 = 15 KiB
         *   residual half tile   7680 * 2 = 15 KiB
         *   full FP16 y cache   32768 * 2 = 64 KiB
         *   value FP32 tile      7680 * 4 = 30 KiB
         *   x FP32 tile          7680 * 4 = 30 KiB
         *   residual FP32 tile   7680 * 4 = 30 KiB
         *   scalar/partial slots            < 1 KiB
         *
         * The generic gamma/bias FP32 caches are intentionally not allocated
         * in this mode because the mode is for one-row-per-core wide work.
         */
        if (wideFp16CachedMode_) {
            tpipe.InitBuffer(
                xBuf_,
                kWideFp16CachedTileElems * sizeof(T));

            tpipe.InitBuffer(
                residualBuf_,
                kWideFp16CachedTileElems * sizeof(T));

            tpipe.InitBuffer(
                outputBuf_,
                rowWidth * sizeof(T));

            tpipe.InitBuffer(
                valueFp32Buf_,
                kWideFp16CachedTileElems * sizeof(float));

            tpipe.InitBuffer(
                xFp32Buf_,
                kWideFp16CachedTileElems * sizeof(float));

            tpipe.InitBuffer(
                residualFp32Buf_,
                kWideFp16CachedTileElems * sizeof(float));

            tpipe.InitBuffer(
                sumBuf_,
                kScalarElems * sizeof(float));

            return;
        }

        /*
         * Parent allocation unchanged.
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

# Dispatch in Process before existing cache logic
needle = """        const uint64_t localRows = endRow > beginRow ? endRow - beginRow : 0ULL;
        const bool useParamCache = rowWidth <= kParamCacheMaxWidth && localRows > 1ULL;
"""
replacement = """        const uint64_t localRows = endRow > beginRow ? endRow - beginRow : 0ULL;

        /*
         * R029-V002 is FP16-only. Keep the call in if constexpr so the
         * low-precision-only method is not instantiated for other dtypes.
         */
        if constexpr (std::is_same<T, half>::value) {
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

        const bool useParamCache =
            rowWidth <= kParamCacheMaxWidth &&
            localRows > 1ULL;
"""
if needle not in text:
    raise RuntimeError("Process dispatch point not found")
text = text.replace(needle, replacement, 1)

# Helper after TileLength
needle = """    __aicore__ inline int32_t TileLength(uint64_t remain) const
    {
        return static_cast<int32_t>(
            remain > static_cast<uint64_t>(kTileElems) ? kTileElems : remain);
    }

"""
replacement = needle + """    __aicore__ inline int32_t WideFp16CachedTileLength(
        uint64_t remain) const
    {
        return static_cast<int32_t>(
            remain > static_cast<uint64_t>(kWideFp16CachedTileElems)
                ? kWideFp16CachedTileElems
                : remain);
    }

"""
if needle not in text:
    raise RuntimeError("TileLength not found")
text = text.replace(needle, replacement, 1)

# Insert new method before PrepareParameterCache
needle = "    __aicore__ inline void PrepareParameterCache(uint64_t rowWidth)\n"
if needle not in text:
    raise RuntimeError("PrepareParameterCache not found")

method = r'''    __aicore__ inline void ProcessWideFp16CachedRow(
        uint64_t row,
        uint64_t rowWidth,
        float invRowWidth,
        float epsilon)
    {
        /*
         * Reached only for T=half.
         *
         * The reduction always uses the full FP32 y value.
         * A compact FP16 copy of y is retained only for the output pass.
         */
        const uint64_t rowOffset = row * rowWidth;

        AscendC::LocalTensor<T> xLocal =
            xBuf_.Get<T>();

        AscendC::LocalTensor<T> residualLocal =
            residualBuf_.Get<T>();

        AscendC::LocalTensor<T> cachedY =
            outputBuf_.Get<T>();

        AscendC::LocalTensor<float> valueFp32 =
            valueFp32Buf_.Get<float>();

        AscendC::LocalTensor<float> xFp32 =
            xFp32Buf_.Get<float>();

        AscendC::LocalTensor<float> residualFp32 =
            residualFp32Buf_.Get<float>();

        AscendC::LocalTensor<float> sumSlots =
            sumBuf_.Get<float>();

        int32_t tileIndex = 0;

        /*
         * Pass 1:
         *   load x/residual once
         *   convert to FP32
         *   form exact FP32 y for RMS
         *   retain a compact FP16 y for the output pass
         */
        for (uint64_t col = 0;
             col < rowWidth;
             col += kWideFp16CachedTileElems) {

            const int32_t valid =
                WideFp16CachedTileLength(
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

            ToFloat(
                xFp32,
                xLocal,
                valid);

            ToFloat(
                residualFp32,
                residualLocal,
                valid);

            AscendC::PipeBarrier<PIPE_V>();

            AscendC::Add(
                valueFp32,
                xFp32,
                residualFp32,
                valid);

            AscendC::PipeBarrier<PIPE_V>();

            /*
             * Cache y in native FP16. This is the only numerical compression
             * introduced by this route.
             */
            AscendC::Cast(
                cachedY[col],
                valueFp32,
                AscendC::RoundMode::CAST_RINT,
                valid);

            AscendC::PipeBarrier<PIPE_V>();

            AscendC::Mul(
                residualFp32,
                valueFp32,
                valueFp32,
                valid);

            AscendC::PipeBarrier<PIPE_V>();

            AscendC::ReduceSum(
                sumSlots[tileIndex],
                residualFp32,
                xFp32,
                valid);

            AscendC::PipeBarrier<PIPE_V>();

            ++tileIndex;

            SyncVToMTE2();
        }

        /*
         * <= 5 partial sums for D <= 32768 with tile 7680.
         */
        AscendC::ReduceSum(
            residualFp32,
            sumSlots,
            xFp32,
            tileIndex);

        AscendC::PipeBarrier<PIPE_V>();

        SyncVToS();

        const float squareSum =
            residualFp32.GetValue(0);

        const float meanSquare =
            squareSum * invRowWidth +
            epsilon;

        SyncSToV();

        AscendC::Duplicate(
            xFp32,
            meanSquare,
            1);

        AscendC::Sqrt(
            xFp32,
            xFp32,
            1);

        AscendC::PipeBarrier<PIPE_V>();

        SyncVToS();

        const float invRms =
            1.0f /
            xFp32.GetValue(0);

        SyncSToV();

        SyncVToMTE2();

        /*
         * Output pass:
         *
         *   cached FP16 y -> FP32
         *   * invRms
         *   * gamma + bias in FP32
         *   final CAST_RINT to FP16
         *
         * No second x/residual GM read.
         */
        for (uint64_t col = 0;
             col < rowWidth;
             col += kWideFp16CachedTileElems) {

            const int32_t valid =
                WideFp16CachedTileLength(
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

            ToFloat(
                valueFp32,
                cachedY[col],
                valid);

            ToFloat(
                xFp32,
                xLocal,
                valid);

            ToFloat(
                residualFp32,
                residualLocal,
                valid);

            AscendC::PipeBarrier<PIPE_V>();

            AscendC::Muls(
                valueFp32,
                valueFp32,
                invRms,
                valid);

            AscendC::PipeBarrier<PIPE_V>();

            AscendC::Mul(
                valueFp32,
                valueFp32,
                xFp32,
                valid);

            AscendC::PipeBarrier<PIPE_V>();

            AscendC::Add(
                valueFp32,
                valueFp32,
                residualFp32,
                valid);

            AscendC::PipeBarrier<PIPE_V>();

            AscendC::Cast(
                cachedY[col],
                valueFp32,
                AscendC::RoundMode::CAST_RINT,
                valid);

            AscendC::PipeBarrier<PIPE_V>();

            SyncVToMTE2();

            SyncVToMTE3();

            Store(
                outputGm_,
                rowOffset + col,
                cachedY[col],
                valid);

            SyncMTE3ToV();
        }
    }

'''
text = text.replace(needle, method + needle, 1)

# Add new buffers and bool before private members
needle = """private:
    AscendC::TPipe tpipe;
"""
replacement = """private:
    bool wideFp16CachedMode_;
    AscendC::TPipe tpipe;
"""
if needle not in text:
    raise RuntimeError("private tpipe point not found")
text = text.replace(needle, replacement, 1)

# Add TBuf declarations after yFp32Buf_
needle = """    AscendC::TBuf<AscendC::TPosition::VECCALC> yFp32Buf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> sumBuf_;
"""
replacement = """    AscendC::TBuf<AscendC::TPosition::VECCALC> yFp32Buf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> outputBuf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> valueFp32Buf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> sumBuf_;
"""
if needle not in text:
    raise RuntimeError("buffer declaration point not found")
text = text.replace(needle, replacement, 1)

# Init call in device kernel
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
    raise RuntimeError("op.Init not found")
text = text.replace(needle, replacement, 1)

# Save
out = Path("/mnt/data/R029-V002-WIDE-FP16-CACHED_kernel.asc")
out.write_text(text, encoding="utf-8")

# Static checks
brace = 0
brace_min = 0
for ch in text:
    if ch == "{": brace += 1
    elif ch == "}": brace -= 1
    brace_min = min(brace_min, brace)

info = {
    "version": "R029-V002-WIDE-FP16-CACHED",
    "parent": "MIX-R014-R002-R019-V001",
    "relation_to_R029_V001": "independent sibling; does not inherit the FP32 wide-cache mode",
    "single_new_mechanism": "FP16 wide-row compact cached-y for 8192 < D <= 32768 and rowCount <= blockCount",
    "sha256": hashlib.sha256(out.read_bytes()).hexdigest(),
    "brace_final": brace,
    "brace_min": brace_min,
    "has_run_kernel": 'extern "C" void run_kernel' in text,
    "has_device_kernel": "__global__ __vector__" in text,
    "compile_status": "not tested in this environment",
    "online_status": "not tested",
}
Path("/mnt/data/R029-V002-WIDE-FP16-CACHED_STATIC.json").write_text(
    json.dumps(info, ensure_ascii=False, indent=2),
    encoding="utf-8"
)

# Package into latest exploration archive
base_pkg = Path("/mnt/data/AddRmsNormBias_ALL_IN_ONE_R011_RESULT_R029_NEXT_v2.zip")
work = Path("/mnt/data/_pkg_r029_v002")
if work.exists():
    shutil.rmtree(work)
work.mkdir()
with zipfile.ZipFile(base_pkg, "r") as z:
    z.extractall(work)

roots = [p for p in work.iterdir() if p.is_dir()]
root = roots[0] if len(roots) == 1 else work
(root/"code"/out.name).write_bytes(out.read_bytes())
(root/"code"/"R029-V002-WIDE-FP16-CACHED_STATIC.json").write_text(
    json.dumps(info, ensure_ascii=False, indent=2),
    encoding="utf-8"
)

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

    MIX19 --> R029A["R029-V001<br/>Wide FP32 Cached Row<br/>待测"]
    MIX19 --> R029B["R029-V002<br/>Wide FP16 Cached Row<br/>下一候选"]

    R005["R005 Large Tile<br/>仅作 Tile/UB 参考"]
    R005 -. "7680 tile" .-> R029A
    R005 -. "7680 tile" .-> R029B
```
"""
(root/"docs"/"当前探索树.md").write_text(tree, encoding="utf-8")

card = """# R029-V002-WIDE-FP16-CACHED 实验卡

Parent：`MIX-R014-R002-R019-V001`

与 `R029-V001` 的关系：
- 独立 sibling；
- 不继承 V001 的 FP32 wide-cache 代码；
- 可以单独测试，避免把两个未验证机制绑在一起。

唯一新增机制：
- dtype=FP16；
- 8192 < D <= 32768；
- `rowCount <= blockCount`；
- 首遍用 FP32 计算 y 与 RMS；
- 同时将 y 压缩保存在 FP16 UB row cache；
- 第二遍只加载 gamma/bias，不再加载 x/residual。

保持不变：
- 小 D 的 R014+R002；
- R019 reciprocal + Muls；
- fallback；
- row scheduling；
- output CAST_RINT。

注意：
- FP16 y cache 会引入一次中间舍入，这是该路线固有风险；
- 线上 15/15 correctness 是接受条件；
- 当前环境未做 CANN compile。
"""
(root/"code"/"R029-V002-WIDE-FP16-CACHED_EXPERIMENT_CARD.md").write_text(
    card, encoding="utf-8"
)

sha_path = root/"SHA256SUMS.txt"
hash_lines = []
for p in sorted(root.rglob("*")):
    if p.is_file() and p != sha_path:
        hash_lines.append(
            f"{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.relative_to(root)}"
        )
sha_path.write_text("\n".join(hash_lines)+"\n", encoding="utf-8")

pkg = Path("/mnt/data/AddRmsNormBias_ALL_IN_ONE_R029_V002_NEXT.zip")
if pkg.exists():
    pkg.unlink()
with zipfile.ZipFile(pkg, "w", zipfile.ZIP_DEFLATED) as z:
    for p in sorted(root.rglob("*")):
        if p.is_file():
            z.write(
                p,
                Path("AddRmsNormBias_ALL_IN_ONE_R029_V002_NEXT") / p.relative_to(root)
            )
with zipfile.ZipFile(pkg) as z:
    bad = z.testzip()

print("source:", out)
print("sha256:", info["sha256"])
print("brace_final:", brace, "brace_min:", brace_min)
print("zip:", pkg)
print("integrity:", "PASS" if bad is None else bad)
