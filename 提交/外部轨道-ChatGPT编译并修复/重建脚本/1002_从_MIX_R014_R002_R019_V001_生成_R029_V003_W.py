# 从 MIX-R014-R002-R019-V001 生成 R029-V003-WIDE-BF16-CACHED
# 来源：ChatGPT 分享对话《编译并修复》节点 1002
# 原始脚本硬编码沙箱路径 /mnt/data/，本机复跑需替换为本地路径。

from pathlib import Path
import re, hashlib, json, zipfile, shutil, math

# ---------- Build next candidate ----------
src = Path("/mnt/data/MIX-R014-R002-R019-V001_kernel.asc")
text = src.read_text(encoding="utf-8")

text = text.replace(
    "AddRmsNormBias - MIX-R014-R002-R019-V001",
    "AddRmsNormBias - R029-V003-WIDE-BF16-CACHED",
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
 *   - add one BF16-only wide-row cached-y runtime mode.
 *   - when 8192 < D <= 32768 and rowCount <= blockCount, retain y in a
 *     compact FP16 UB row cache while computing RMS from the original FP32 y.
 *   - the output pass then reloads gamma/bias only; x/residual are not read
 *     from GM a second time.
 *
 * Existing mechanisms retained:
 *   - R014-V002 parameter residency for small rows
 *   - R002 retained-y small-row path
 *   - R019 scalar reciprocal + vector Muls normalization
 *
 * This is an independent sibling of R029-V001/V002 and does not depend on
 * their acceptance.
""",
1)

text = text.replace(
    "    static constexpr int32_t kScalarElems = 16;\n",
"""    static constexpr int32_t kScalarElems = 16;
    static constexpr int32_t kWideBf16CachedTileElems = 7680;
    static constexpr uint64_t kWideBf16CachedRowMaxWidth = 32768ULL;
""",
1)

# Shape-aware Init
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

        wideBf16CachedMode_ = false;

        if constexpr (AscendC::IsSameType<T, bfloat16_t>::value) {
            wideBf16CachedMode_ =
                rowWidth > static_cast<uint64_t>(kTileElems) &&
                rowWidth <= kWideBf16CachedRowMaxWidth &&
                rowCount <= static_cast<uint64_t>(blockCount);
        }

        /*
         * R029-V003 BF16-only wide cached-row mode.
         *
         * At D=32768:
         *   x BF16 tile           7680 * 2 = 15 KiB
         *   residual BF16 tile    7680 * 2 = 15 KiB
         *   full FP16 y cache    32768 * 2 = 64 KiB
         *   value FP32 tile       7680 * 4 = 30 KiB
         *   x FP32 tile           7680 * 4 = 30 KiB
         *   residual FP32 tile    7680 * 4 = 30 KiB
         *   scalar/partial slots             < 1 KiB
         *
         * Main payload ~= 184 KiB. The generic parameter FP32 caches are
         * intentionally not allocated in this path.
         */
        if (wideBf16CachedMode_) {
            tpipe.InitBuffer(
                xBuf_,
                kWideBf16CachedTileElems * sizeof(T));

            tpipe.InitBuffer(
                residualBuf_,
                kWideBf16CachedTileElems * sizeof(T));

            /*
             * outputBuf_ is physically 16-bit per element. During pass 1 it
             * is viewed as LocalTensor<half> for the compact y cache. During
             * the output pass the same storage is viewed as LocalTensor<T>.
             */
            tpipe.InitBuffer(
                outputBuf_,
                rowWidth * sizeof(T));

            tpipe.InitBuffer(
                valueFp32Buf_,
                kWideBf16CachedTileElems * sizeof(float));

            tpipe.InitBuffer(
                xFp32Buf_,
                kWideBf16CachedTileElems * sizeof(float));

            tpipe.InitBuffer(
                residualFp32Buf_,
                kWideBf16CachedTileElems * sizeof(float));

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

# Dispatch
needle = """        const uint64_t localRows = endRow > beginRow ? endRow - beginRow : 0ULL;
        const bool useParamCache = rowWidth <= kParamCacheMaxWidth && localRows > 1ULL;
"""
replacement = """        const uint64_t localRows = endRow > beginRow ? endRow - beginRow : 0ULL;

        if constexpr (AscendC::IsSameType<T, bfloat16_t>::value) {
            if (wideBf16CachedMode_) {
                for (uint64_t row = beginRow; row < endRow; ++row) {
                    ProcessWideBf16CachedRow(
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
    raise RuntimeError("dispatch point not found")
text = text.replace(needle, replacement, 1)

# Helper
needle = """    __aicore__ inline int32_t TileLength(uint64_t remain) const
    {
        return static_cast<int32_t>(
            remain > static_cast<uint64_t>(kTileElems) ? kTileElems : remain);
    }

"""
replacement = needle + """    __aicore__ inline int32_t WideBf16CachedTileLength(
        uint64_t remain) const
    {
        return static_cast<int32_t>(
            remain > static_cast<uint64_t>(kWideBf16CachedTileElems)
                ? kWideBf16CachedTileElems
                : remain);
    }

"""
if needle not in text:
    raise RuntimeError("TileLength block not found")
text = text.replace(needle, replacement, 1)

# New BF16 path
needle = "    __aicore__ inline void PrepareParameterCache(uint64_t rowWidth)\n"
if needle not in text:
    raise RuntimeError("PrepareParameterCache insertion point not found")

method = r'''    __aicore__ inline void ProcessWideBf16CachedRow(
        uint64_t row,
        uint64_t rowWidth,
        float invRowWidth,
        float epsilon)
    {
        /*
         * Reached only for T=bfloat16_t.
         *
         * RMS is computed from the original FP32 y. The retained row cache is
         * FP16 rather than BF16 because both consume 16 bits while FP16 keeps
         * more mantissa bits for values in the representable range.
         */
        const uint64_t rowOffset = row * rowWidth;

        AscendC::LocalTensor<T> xLocal =
            xBuf_.Get<T>();

        AscendC::LocalTensor<T> residualLocal =
            residualBuf_.Get<T>();

        AscendC::LocalTensor<half> cachedY =
            outputBuf_.Get<half>();

        AscendC::LocalTensor<T> outputLocal =
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
         *   BF16 x/residual -> FP32
         *   y = x + residual in FP32
         *   RMS reduction from FP32 y
         *   compact copy of y retained as FP16
         */
        for (uint64_t col = 0;
             col < rowWidth;
             col += kWideBf16CachedTileElems) {

            const int32_t valid =
                WideBf16CachedTileLength(
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
             * This cache is for the second pass only. The RMS reduction below
             * still sees the original FP32 y.
             */
            AscendC::Cast(
                cachedY[col],
                valueFp32,
                AscendC::RoundMode::CAST_ROUND,
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
         * D <= 32768 and tile=7680 => tileIndex <= 5.
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
         *   FP16 cached y -> FP32
         *   normalize in FP32
         *   BF16 gamma/bias -> FP32
         *   final BF16 CAST_RINT
         *
         * No second x/residual GM read.
         */
        for (uint64_t col = 0;
             col < rowWidth;
             col += kWideBf16CachedTileElems) {

            const int32_t valid =
                WideBf16CachedTileLength(
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

            AscendC::Cast(
                valueFp32,
                cachedY[col],
                AscendC::RoundMode::CAST_NONE,
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
                outputLocal[col],
                valueFp32,
                AscendC::RoundMode::CAST_RINT,
                valid);

            AscendC::PipeBarrier<PIPE_V>();

            /*
             * cachedY[col] has now been consumed, so overwriting the same
             * 16-bit storage with BF16 output is safe.
             */
            SyncVToMTE2();

            SyncVToMTE3();

            Store(
                outputGm_,
                rowOffset + col,
                outputLocal[col],
                valid);

            SyncMTE3ToV();
        }
    }

'''
text = text.replace(needle, method + needle, 1)

# New members/buffers
text = text.replace(
"""private:
    AscendC::TPipe tpipe;
""",
"""private:
    bool wideBf16CachedMode_;
    AscendC::TPipe tpipe;
""",
1)

text = text.replace(
"""    AscendC::TBuf<AscendC::TPosition::VECCALC> yFp32Buf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> sumBuf_;
""",
"""    AscendC::TBuf<AscendC::TPosition::VECCALC> yFp32Buf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> outputBuf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> valueFp32Buf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> sumBuf_;
""",
1)

# Shape-aware Init call
text = text.replace(
"    op.Init(x, residual, gamma, bias, output);\n",
"""    op.Init(
        x,
        residual,
        gamma,
        bias,
        output,
        rowWidth,
        rowCount,
        blockCount);
""",
1)

out = Path("/mnt/data/R029-V003-WIDE-BF16-CACHED_kernel.asc")
out.write_text(text, encoding="utf-8")

# ---------- Static checks ----------
brace = 0
brace_min = 0
for ch in text:
    if ch == "{":
        brace += 1
    elif ch == "}":
        brace -= 1
    brace_min = min(brace_min, brace)

static = {
    "version": "R029-V003-WIDE-BF16-CACHED",
    "parent": "MIX-R014-R002-R019-V001",
    "single_new_mechanism": "BF16 wide-row FP16 cached-y path",
    "activation": "dtype=BF16, 8192 < D <= 32768, rowCount <= blockCount",
    "sha256": hashlib.sha256(out.read_bytes()).hexdigest(),
    "brace_final": brace,
    "brace_min": brace_min,
    "has_run_kernel": 'extern "C" void run_kernel' in text,
    "has_device_kernel": "__global__ __vector__" in text,
    "compile_status": "not tested in this environment",
    "online_status": "not tested"
}
static_path = Path("/mnt/data/R029-V003-WIDE-BF16-CACHED_STATIC.json")
static_path.write_text(json.dumps(static, ensure_ascii=False, indent=2), encoding="utf-8")

# ---------- Latest R029-V002 result metrics ----------
baseline = [
    4.33,11.96,8.98,97.36,14.02,122.74,239.20,401.84,191.36,
    323.92,258.44,251.22,758.78,125000.0,11300.0
]
current = [
    4.34,8.79,6.50,32.16,13.26,49.68,77.42,197.83,111.43,
    182.42,181.30,129.67,611.82,49800.0,10800.0
]
best = [
    1.47,2.16,2.48,6.66,5.21,11.66,14.05,30.64,50.61,
    47.35,67.54,76.68,307.60,3750.0,8510.0
]
improvement = (sum(baseline)-sum(current))/sum(baseline)*100.0
score = sum(min(1.0,b/t) for b,t in zip(best,current))/15*100.0

# ---------- Record/package ----------
base_pkg = Path("/mnt/data/AddRmsNormBias_ALL_IN_ONE_R029_V002_NEXT.zip")
work = Path("/mnt/data/_pkg_r029_v002_result_v003_next")
if work.exists():
    shutil.rmtree(work)
work.mkdir()
with zipfile.ZipFile(base_pkg, "r") as z:
    z.extractall(work)

roots = [p for p in work.iterdir() if p.is_dir()]
root = roots[0] if len(roots) == 1 else work

(root/"code"/out.name).write_bytes(out.read_bytes())
(root/"code"/static_path.name).write_bytes(static_path.read_bytes())

record = f"""# R029-V002 在线结果与 R029-V003 下一版本

## R029-V002-WIDE-FP16-CACHED

- Correctness: 15/15 Pass
- 当前总耗时: {sum(current):.2f} μs
- 相对既定用户基线总耗时提升: {improvement:.2f}%
- 按当前 best/current 等权口径复算分数: {score:.2f}
- 结论: 当前新的实测最好版本。

15 Case:
`4.34, 8.79, 6.50, 32.16, 13.26, 49.68, 77.42, 197.83, 111.43, 182.42, 181.30, 129.67, 611.82, 49.8ms, 10.8ms`

## 下一候选

`R029-V003-WIDE-BF16-CACHED`

Parent:
`MIX-R014-R002-R019-V001`

独立 sibling，不叠加尚未验证的 R029-V001。

唯一新增：
- BF16
- 8192 < D <= 32768
- rowCount <= blockCount
- RMS 仍由原始 FP32 y 计算
- y 额外缓存为 FP16，以 16-bit UB 成本保留更多尾数
- 第二遍不再读取 x/residual
"""
(root/"docs"/"R029-V002结果与R029-V003计划.md").write_text(record, encoding="utf-8")

tree = f"""# 当前探索树

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
    MIX --> MIX19["R014 + R002 + R019"]

    D --> R011["R011 Manual Reduction"]
    R011 --> X11["Performance Reject"]

    MIX19 --> R029V1["R029-V001<br/>Wide FP32 Cached<br/>待测"]
    MIX19 --> R029V2["R029-V002<br/>Wide FP16 Cached<br/>15/15<br/>{improvement:.2f}% / {score:.2f}<br/>当前最佳"]
    MIX19 --> R029V3["R029-V003<br/>Wide BF16 Cached<br/>下一候选"]

    R005["R005 Large Tile<br/>仅作 Tile/UB 参考"]
    R005 -. "7680 Tile" .-> R029V1
    R005 -. "7680 Tile" .-> R029V2
    R005 -. "7680 Tile" .-> R029V3
```
"""
(root/"docs"/"当前探索树.md").write_text(tree, encoding="utf-8")

sha_path = root/"SHA256SUMS.txt"
hash_lines = []
for p in sorted(root.rglob("*")):
    if p.is_file() and p != sha_path:
        hash_lines.append(f"{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.relative_to(root)}")
sha_path.write_text("\n".join(hash_lines)+"\n", encoding="utf-8")

pkg = Path("/mnt/data/AddRmsNormBias_ALL_IN_ONE_R029_V002_RESULT_V003_NEXT.zip")
if pkg.exists():
    pkg.unlink()
with zipfile.ZipFile(pkg, "w", zipfile.ZIP_DEFLATED) as z:
    for p in sorted(root.rglob("*")):
        if p.is_file():
            z.write(
                p,
                Path("AddRmsNormBias_ALL_IN_ONE_R029_V002_RESULT_V003_NEXT") / p.relative_to(root)
            )

with zipfile.ZipFile(pkg) as z:
    bad = z.testzip()

print(f"R029-V002 baseline improvement: {improvement:.2f}%")
print(f"R029-V002 score recalc: {score:.2f}")
print(f"R029-V003 source: {out}")
print(f"R029-V003 SHA256: {static['sha256']}")
print(f"Static brace check: final={brace}, min={brace_min}")
print(f"Package: {pkg}")
print(f"ZIP integrity: {'PASS' if bad is None else bad}")
