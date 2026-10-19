#!/usr/bin/env bash
set -uo pipefail
POLL_LOG=/home/data4t2/lelinfeng/phase4-workspaces/ALIGN-TAIL-X/probe/load-poll-window2.log
mkdir -p "$(dirname "$POLL_LOG")"
echo "START $(date -Is)" >> "$POLL_LOG"

is_clean() {
  python3 - <<'PY'
import re, subprocess, sys
smi = subprocess.check_output(["npu-smi","info"], text=True, errors="replace")
lines = smi.splitlines()
devs = []
for i, line in enumerate(lines):
    m = re.match(r'\|\s*(\d+)\s+910B3', line)
    if m and i + 1 < len(lines):
        dev = int(m.group(1))
        m2 = re.match(
            r'\|\s*\d+\s+\|\s*[0-9A-Fa-f:.]+\s*\|\s*(\d+)\s+\|\s*\d+\s*/\s*\d+\s+(\d+)\s*/\s*(\d+)',
            lines[i+1])
        if m2:
            aicore = int(m2.group(1))
            used = int(m2.group(2))
            total = int(m2.group(3))
            devs.append((dev, aicore, used, total))
procs = []
for line in lines:
    m = re.match(r'\|\s*(\d+)\s+(\d+)\s+\|\s+(\d+)\s+\|\s+([A-Za-z0-9_.\-]+)', line)
    if m:
        procs.append((int(m.group(1)), int(m.group(3)), m.group(4)))
print("DEVS", devs)
print("PROCS", procs)
if not devs:
    print("PARSE_FAIL")
    sys.exit(1)

# System-wide pressure: any VLLM/Engine/Worker/python process, or any AICore>0 with those procs
pressure = [(d, pid, name) for d, pid, name in procs
            if re.search(r'VLLM|Engine|Worker|python|vllm', name, re.I)]
active_aicore = [(d, a) for d, a, u, t in devs if a > 0]
hbm_hot = [(d, u, t) for d, a, u, t in devs if t and u / t > 0.90]
print("PRESSURE_PROCS", pressure)
print("ACTIVE_AICORE", active_aicore)
print("HBM_HOT", hbm_hot)

# Main criteria: no VLLM/python pressure, HBM not saturated
# Clean = no pressure processes at all AND no active aicore from them AND max hbm frac <= 0.90
max_frac = max((u / t) for d, a, u, t in devs if t) if devs else 1.0
if pressure or active_aicore or max_frac > 0.90:
    print(f"NOT_CLEAN max_frac={max_frac:.3f}")
    open("/tmp/atx_clean_dev.txt", "w").write("")
    sys.exit(1)

# All quiet: pick lowest aicore then lowest hbm device for probes
devs_sorted = sorted(devs, key=lambda x: (x[1], x[2]))
dev = devs_sorted[0][0]
print("CLEAN_READY", dev, "max_frac", f"{max_frac:.3f}")
open("/tmp/atx_clean_dev.txt", "w").write(str(dev))
sys.exit(0)
PY
}

for i in $(seq 1 40); do
  ts=$(date -Is)
  echo "===== POLL $i $ts =====" >> "$POLL_LOG"
  if is_clean 2>&1 | tee -a "$POLL_LOG"; then
    sleep 10
    if is_clean >/dev/null 2>&1; then
      dev=$(cat /tmp/atx_clean_dev.txt)
      echo "CLEAN_WINDOW dev=$dev at $(date -Is)" | tee -a "$POLL_LOG"
      echo "$dev" > /tmp/atx_clean_dev_ready.txt
      # snapshot
      npu-smi info > /home/data4t2/lelinfeng/phase4-workspaces/ALIGN-TAIL-X/probe/results-window2-npu-smi-pre.txt 2>/dev/null || true
      exit 0
    fi
    echo "VERIFY_FAILED" >> "$POLL_LOG"
  fi
  sleep 45
done
echo "NO_CLEAN_WINDOW after 40 polls" | tee -a "$POLL_LOG"
exit 2
