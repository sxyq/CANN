#!/usr/bin/env bash
set -eo pipefail
source /usr/local/Ascend/ascend-toolkit/set_env.sh
cd /home/data4t2/lelinfeng/server_runs/W4-R14/param-mte2-fingerprint-20261008/parent-probe
umask 027
for r14_case in '2 8192 0' '3 6144 0' '2 8192 1' '3 6144 1' '80 8192 0' '120 6144 0' '1 32768 0'; do
    read -r r14_rows r14_width r14_cores <<< "$r14_case"
    r14_prefix="results/same_${r14_rows}x${r14_width}_c${r14_cores}"
    npu-smi info -t usages -i 0 > "$r14_prefix.pre.npu.txt"
    cat /proc/loadavg > "$r14_prefix.pre.load.txt"
    r14_free_mb=$(awk '/HBM Capacity\(MB\)/{cap=$NF} /^[[:space:]]*HBM Usage Rate\(%\)/{used=$NF} END{if(cap=="")exit 1; print int(cap*(100-used)/100)}' "$r14_prefix.pre.npu.txt")
    printf 'PARENT rows=%s width=%s requested_cores=%s DEVICE_ID=0 FREE_HBM_MB=%s\n' "$r14_rows" "$r14_width" "$r14_cores" "$r14_free_mb"
    test "$r14_free_mb" -ge 100
    ./build/r14_parent_probe 0 "$r14_rows" "$r14_width" "$r14_cores" 45 21 2 > "$r14_prefix.tsv" 2> "$r14_prefix.stderr.txt"
    cat "$r14_prefix.stderr.txt"
    npu-smi info -t usages -i 0 > "$r14_prefix.post.npu.txt"
    cat /proc/loadavg > "$r14_prefix.post.load.txt"
done
