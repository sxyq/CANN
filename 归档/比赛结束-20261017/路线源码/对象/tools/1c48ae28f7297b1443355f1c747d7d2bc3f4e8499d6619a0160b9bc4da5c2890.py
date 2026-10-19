"""Read the existing R12 Parent executable on server3 without rebuilding it."""

from datetime import datetime, timezone
from pathlib import Path
import os
import re
import shlex
import struct
import subprocess
import sys


binary = Path(
    "/home/data4t2/lelinfeng/server_runs/W4-R12/parent-tiny-20261008/"
    "build/r12_parent_probe"
)
objdump = (
    "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/"
    "compiler/ccec_compiler/bin/llvm-objdump"
)
symbol = "_Z24add_rms_norm_bias_customIfEvPhS0_S0_S0_S0_mmjff"
if sys.argv[1:] in (["--emit-assembly"], ["--save-intermediates"], ["--driver-plan"], ["--device-plan"], ["--machine-output"]):
    root = binary.parent.parent
    save_intermediates = sys.argv[1] == "--save-intermediates"
    print("ACTION=unchanged Parent compiler output", flush=True)
    print("PARENT_EXECUTABLE=" + str(binary), flush=True)
    print("PARENT_MTIME_NS=" + str(binary.stat().st_mtime_ns), flush=True)
    print("UTC=" + datetime.now(timezone.utc).isoformat(), flush=True)
    usage = subprocess.check_output(["npu-smi", "info", "-t", "usages", "-i", "4"], text=True)
    print(usage, flush=True)
    capacity = int(re.search(r"HBM Capacity\(MB\)\s*:\s*(\d+)", usage).group(1))
    percent = int(re.search(r"(?m)^\s*HBM Usage Rate\(%\)\s*:\s*(\d+)", usage).group(1))
    free_mb = capacity * (100 - percent) // 100
    print(f"FREE_HBM_MB={free_mb}", flush=True)
    assert free_mb >= 100
    print("LOAD=" + Path("/proc/loadavg").read_text().strip(), flush=True)
    processes = subprocess.check_output(["ps", "-eo", "pid,args"], text=True).splitlines()
    print("R12_ACTIVE_PROCESSES_BEFORE=" + repr([p for p in processes if str(root) in p]), flush=True)
    cmake_dir = binary.parent / "CMakeFiles/r12_parent_probe.dir"
    flags = {}
    for line in (cmake_dir / "flags.make").read_text().splitlines():
        if line.startswith("ASC_"):
            key, value = line.split("=", 1)
            flags[key.strip()] = value.strip()
    recipe = next(line.strip() for line in (cmake_dir / "build.make").read_text().splitlines()
                  if line.strip().startswith(objdump.rsplit("/", 1)[0] + "/bisheng "))
    for key, value in flags.items():
        recipe = recipe.replace("$(" + key + ")", value)
    original = shlex.split(recipe)
    output_dir = root / "code-study"
    assert not list(output_dir.glob("*.s"))
    assert not (output_dir / "parent-inspect.o").exists()
    output_dir.mkdir(exist_ok=True)
    command = []
    i = 0
    while i < len(original):
        item = original[i]
        if item in ("-MT", "-MF", "-o"):
            i += 2
            continue
        if item not in ("-MD", "-c"):
            command.append(item)
        i += 1
    if sys.argv[1] == "--machine-output":
        insert_at = command.index("-Xaicore-end")
        command[insert_at:insert_at] = ["-mllvm", "-print-after=prologepilog",
                                      "-mllvm", "-filter-print-funcs=" + symbol]
        command += ["-c", "-o", str(output_dir / "parent-inspect.o")]
    elif sys.argv[1] in ("--driver-plan", "--device-plan"):
        command += ["-c", "-o", str(output_dir / "parent-inspect.o"), "-###"]
        if sys.argv[1] == "--device-plan":
            command += ["--merge-bisheng-plugin-output=" + str(output_dir / "bisheng-plugin-output-a51952.json")]
    elif save_intermediates:
        command += ["-c", "-save-temps", "-o", str(output_dir / "parent-inspect.o")]
    else:
        command += ["--cce-aicore-only", "-S"]
    print("COMMAND=" + shlex.join(command), flush=True)
    env = os.environ.copy()
    env["TMPDIR"] = str(output_dir)
    result = subprocess.run(command, cwd=output_dir, env=env, capture_output=True, text=True, check=False)
    print(result.stdout, flush=True)
    print(result.stderr, flush=True)
    print("COMPILER_RC=" + str(result.returncode), flush=True)
    print("COMPILER_FILES=" + repr([str(p) for p in sorted(output_dir.iterdir())]), flush=True)
    print("PARENT_MTIME_NS_AFTER=" + str(binary.stat().st_mtime_ns), flush=True)
    print("UTC_END=" + datetime.now(timezone.utc).isoformat(), flush=True)
    print("RUNNING_DEVICE_OPERATION=NONE", flush=True)
    raise SystemExit(result.returncode)

data = binary.read_bytes()
assert data[:6] == b"\x7fELF\x02\x01"
header = struct.unpack_from("<16sHHIQQQIHHHHHH", data)
sections = [
    struct.unpack_from("<IIQQQQIIQQ", data, header[6] + i * header[11])
    for i in range(header[12])
]
names_section = sections[header[13]]
names = data[names_section[4] : names_section[4] + names_section[5]]
device_section = next(
    section
    for section in sections
    if names[section[0] :].split(b"\0", 1)[0] == b".aicore_binary"
)
device_data = data[
    device_section[4] : device_section[4] + device_section[5]
]
assert device_data[:6] == b"\x7fELF\x02\x01"
print(f"PARENT_EXECUTABLE={binary}")
print(f"PARENT_BYTES={len(data)}")
print(
    "PARENT_MTIME_UTC="
    + datetime.fromtimestamp(binary.stat().st_mtime, timezone.utc).isoformat()
)
print("SECTION=.aicore_binary")
print(f"SECTION_OFFSET={device_section[4]:#x}")
print(f"SECTION_BYTES={len(device_data)}")
print(f"SYMBOL={symbol}")
print("CPU=dav-c220-vec (DeviceVecExtraCompileOptions from native compiler output)")
print("METHOD=existing linked device ELF through anonymous memory fd")
fd = os.memfd_create("r12-parent-device-code")
try:
    os.write(fd, device_data)
    result = subprocess.run(
        [objdump, "--disassemble-aicore", "--mcpu=dav-c220-vec", "--no-show-raw-insn", f"--disassemble-symbols={symbol}",
         f"/proc/self/fd/{fd}"],
        pass_fds=(fd,), capture_output=True, text=True, check=False,
    )
    print(result.stdout)
    if result.stderr:
        print(result.stderr)
    print(f"OBJDUMP_RC={result.returncode}")
    raise SystemExit(result.returncode)
finally:
    os.close(fd)
