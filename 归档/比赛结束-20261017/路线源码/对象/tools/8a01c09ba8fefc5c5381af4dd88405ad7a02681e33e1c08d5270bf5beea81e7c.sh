#!/usr/bin/env bash
set -uo pipefail
POLL_LOG=/home/data4t2/lelinfeng/phase4-workspaces/ALIGN-TAIL-X/probe/load-poll-window2.log
echo "START2 $(date -Is)" >> "$POLL_LOG"

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
        # chip line: | chip | bus | aicore  mem_used/ mem_tot  hbm_used/ hbm_tot |
        m2 = re.match(
            r'\|\s*\d+\s+\|\s*[0-9A-Fa-f:.]+\s*\|\s*(\d+)\s+(\d+)\s*/\s*(\d+)\s+(\d+)\s*/\s*(\d+)',
            lines[i+1])
        if m2:
            aicore = int(m2.group(1))
            hbm_used = int(m2.group(4))
            hbm_total = int(m2.group(5))
            devs.append((dev, aicore, hbm_used, hbm_total))
procs = []
for line in lines:
    m = re.match(r'\|\s*(\d+)\s+(\d+)\s+\|\s+(\d+)\s+\|\s+([A-Za-z0-9_.\-]+)', line)
    if m:
        procs.append((int(m.group(1)), int(m.group(3)), m.group(4)))
print("DEVS", devs)
print("PROCS", procs)
if len(devs) < 8:
    print("PARSE_FAIL n_devs=", len(devs))
    open("/tmp/atx_clean_dev.txt","w").write("")
    sys.exit(1)

pressure_names = [p for p in procs if re.search(r'VLLM|Engine|Worker|python|vllm', p[2], re.I)]
# Active pressure: VLLM/Engine/python process with device AICore>0, OR any device aicore>0 co-located with those names
active = []
for d, a, u, t in devs:
    names = [p[2] for p in procs if p[0]==d and re.search(r'VLLM|Engine|Worker|python|vllm', p[2], re.I)]
    if names and a > 0:
        active.append((d, a, names))
max_frac = max((u/t) for _,_,u,t in devs if t)
print("ACTIVE_PRESSURE", active)
print("MAX_HBM_FRAC", f"{max_frac:.4f}")
print("PRESSURE_PROCS_COUNT", len(pressure_names))

# Strict clean for Main:
# - no VLLM/Engine/Worker/python process on any device
# - all AICore == 0
# - max HBM frac <= 0.90
if pressure_names:
    print("NOT_CLEAN reason=pressure_procs")
    open("/tmp/atx_clean_dev.txt","w").write("")
    sys.exit(1)
if any(a>0 for _,a,_,_ in devs):
    print("NOT_CLEAN reason=aicore")
    open("/tmp/atx_clean_dev.txt","w").write("")
    sys.exit(1)
if max_frac > 0.90:
    print("NOT_CLEAN reason=hbm")
    open("/tmp/atx_clean_dev.txt","w").write("")
    sys.exit(1)
dev = sorted(devs, key=lambda x: (x[1], x[2]))[0][0]
print("CLEAN_READY", dev)
open("/tmp/atx_clean_dev.txt","w").write(str(dev))
sys.exit(0)
PY
}

for i in $(seq 1 50); do
  ts=$(date -Is)
  echo "===== POLL2 $i $ts =====" >> "$POLL_LOG"
  if is_clean 2>&1 | tee -a "$POLL_LOG"; then
    sleep 12
    if is_clean >/dev/null 2>&1; then
      dev=$(cat /tmp/atx_clean_dev.txt)
      echo "CLEAN_WINDOW dev=$dev at $(date -Is)" | tee -a "$POLL_LOG"
      echo "$dev" > /tmp/atx_clean_dev_ready.txt
      mkdir -p /home/data4t2/lelinfeng/phase4-workspaces/ALIGN-TAIL-X/probe/results-window2
      npu-smi info > /home/data4t2/lelinfeng/phase4-workspaces/ALIGN-TAIL-X/probe/results-window2/npu-smi-pre.txt
      exit 0
    fi
    echo "VERIFY_FAILED" >> "$POLL_LOG"
  fi
  sleep 40
done
echo "NO_CLEAN_WINDOW after 50 more polls" | tee -a "$POLL_LOG"
exit 2
