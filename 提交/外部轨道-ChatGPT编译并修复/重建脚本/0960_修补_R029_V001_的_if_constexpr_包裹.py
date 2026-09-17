# 修补 R029-V001 的 if constexpr 包裹
# 来源：ChatGPT 分享对话《编译并修复》节点 0960
# 原始脚本硬编码沙箱路径 /mnt/data/，本机复跑需替换为本地路径。

from pathlib import Path
import hashlib, json, zipfile, shutil

p = Path("/mnt/data/R029-V001-WIDE-FP32-CACHED_kernel.asc")
text = p.read_text(encoding="utf-8")

old = """        /*
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

"""
new = """        /*
         * R029-V001: one new runtime mode only.
         *
         * Keep the call itself inside if constexpr so the FP32-only member
         * is never instantiated for half/bfloat16 template specializations.
         */
        if constexpr (AscendC::IsSameType<T, float>::value) {
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
        }

"""
if old not in text:
    raise RuntimeError("dispatch block not found")
text = text.replace(old, new, 1)
p.write_text(text, encoding="utf-8")

brace = 0
brace_min = 0
for ch in text:
    if ch == "{": brace += 1
    elif ch == "}": brace -= 1
    brace_min = min(brace_min, brace)

static_path = Path("/mnt/data/R029-V001-WIDE-FP32-CACHED_STATIC.json")
static = json.loads(static_path.read_text(encoding="utf-8"))
static["sha256"] = hashlib.sha256(p.read_bytes()).hexdigest()
static["brace_final"] = brace
static["brace_min"] = brace_min
static["fp32_only_dispatch_guard"] = True
static_path.write_text(json.dumps(static, ensure_ascii=False, indent=2), encoding="utf-8")

# Patch latest package copy and rebuild.
old_pkg = Path("/mnt/data/AddRmsNormBias_ALL_IN_ONE_R011_RESULT_R029_NEXT.zip")
work = Path("/mnt/data/_repack_r029_fix")
if work.exists():
    shutil.rmtree(work)
work.mkdir()
with zipfile.ZipFile(old_pkg, "r") as z:
    z.extractall(work)
roots = [x for x in work.iterdir() if x.is_dir()]
root = roots[0] if len(roots)==1 else work

(root/"code"/p.name).write_bytes(p.read_bytes())
(root/"code"/static_path.name).write_bytes(static_path.read_bytes())

sha = root/"SHA256SUMS.txt"
lines=[]
for f in sorted(root.rglob("*")):
    if f.is_file() and f != sha:
        lines.append(f"{hashlib.sha256(f.read_bytes()).hexdigest()}  {f.relative_to(root)}")
sha.write_text("\n".join(lines)+"\n",encoding="utf-8")

out = Path("/mnt/data/AddRmsNormBias_ALL_IN_ONE_R011_RESULT_R029_NEXT_v2.zip")
if out.exists(): out.unlink()
with zipfile.ZipFile(out,"w",zipfile.ZIP_DEFLATED) as z:
    for f in sorted(root.rglob("*")):
        if f.is_file():
            z.write(f, Path("AddRmsNormBias_ALL_IN_ONE_R011_RESULT_R029_NEXT_v2")/f.relative_to(root))
with zipfile.ZipFile(out) as z:
    bad=z.testzip()

print("R029 SHA256:", static["sha256"])
print("brace:", brace, "min:", brace_min)
print("zip:", out)
print("integrity:", "PASS" if bad is None else bad)
