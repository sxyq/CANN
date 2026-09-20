# 修正 V004 的 private 成员声明顺序
# 来源：ChatGPT 分享对话《编译并修复》节点 1073
# 原始脚本硬编码沙箱路径 /mnt/data/，本机复跑需替换为本地路径。

from pathlib import Path
import hashlib, json, zipfile, shutil

p = Path("/mnt/data/R029-V004-WIDE-LOWP-CACHED_kernel.asc")
text = p.read_text(encoding="utf-8")

old = """private:
    bool wideBf16CachedMode_;
    AscendC::TPipe tpipe;
"""
new = """private:
    bool wideFp16CachedMode_;
    bool wideBf16CachedMode_;
    AscendC::TPipe tpipe;
"""
if old not in text:
    raise RuntimeError("private bool block not found")
text = text.replace(old, new, 1)
p.write_text(text, encoding="utf-8")

# Update static
brace=0; brace_min=0
for ch in text:
    if ch=="{": brace+=1
    elif ch=="}": brace-=1
    brace_min=min(brace_min,brace)

sp = Path("/mnt/data/R029-V004-WIDE-LOWP-CACHED_STATIC.json")
st = json.loads(sp.read_text(encoding="utf-8"))
st["sha256"] = hashlib.sha256(p.read_bytes()).hexdigest()
st["brace_final"]=brace
st["brace_min"]=brace_min
st["has_fp16_mode_member"]="bool wideFp16CachedMode_;" in text
st["has_bf16_mode_member"]="bool wideBf16CachedMode_;" in text
sp.write_text(json.dumps(st,ensure_ascii=False,indent=2),encoding="utf-8")

# Repatch package from existing package
oldpkg=Path("/mnt/data/AddRmsNormBias_ALL_IN_ONE_R029_V003_RESULT_V004_NEXT.zip")
work=Path("/mnt/data/_pkg_fix_v004")
if work.exists(): shutil.rmtree(work)
work.mkdir()
with zipfile.ZipFile(oldpkg) as z: z.extractall(work)
roots=[x for x in work.iterdir() if x.is_dir()]
root=roots[0] if len(roots)==1 else work
(root/"code"/p.name).write_bytes(p.read_bytes())
(root/"code"/sp.name).write_bytes(sp.read_bytes())

# Update references to SHA in current state if any
state_path=root/"handoff"/"CURRENT_STATE.json"
if state_path.exists():
    state=json.loads(state_path.read_text(encoding="utf-8"))
    if isinstance(state.get("next_candidate"),dict):
        state["next_candidate"]["source_sha256"]=st["sha256"]
    state_path.write_text(json.dumps(state,ensure_ascii=False,indent=2),encoding="utf-8")

sha=root/"SHA256SUMS.txt"
lines=[]
for f in sorted(root.rglob("*")):
    if f.is_file() and f!=sha:
        lines.append(f"{hashlib.sha256(f.read_bytes()).hexdigest()}  {f.relative_to(root)}")
sha.write_text("\n".join(lines)+"\n",encoding="utf-8")

newpkg=Path("/mnt/data/AddRmsNormBias_ALL_IN_ONE_R029_V003_RESULT_V004_NEXT_v2.zip")
if newpkg.exists(): newpkg.unlink()
with zipfile.ZipFile(newpkg,"w",zipfile.ZIP_DEFLATED) as z:
    for f in sorted(root.rglob("*")):
        if f.is_file():
            z.write(f,Path("AddRmsNormBias_ALL_IN_ONE_R029_V003_RESULT_V004_NEXT_v2")/f.relative_to(root))
with zipfile.ZipFile(newpkg) as z: bad=z.testzip()

print(st)
print(newpkg,bad)
