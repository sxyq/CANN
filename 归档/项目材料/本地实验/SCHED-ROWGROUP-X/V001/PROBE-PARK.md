# SCHED-ROWGROUP-X V001 — Probe park (Main review r2 → r3 attempt)

## Decision
- V001 source unchanged (SHA 0fae0a42e3942356fe3477cd518b17180b6104d57350cee705cba37a5895e65c).
- CORRECTNESS remains PASS (from prior local matrix).
- **Probes PARKED** — no set-3 run.
- Handoff: **NEEDS_ONE_MORE_LOCAL** with **PARK recommendation for probes**.

## Historical reason this probe was parked
The earlier run required VLLM to stop or required unusually high free HBM. That resource policy was over-conservative and is retired. Current timing admission is live HBM utilization below 100% plus an unconflicted lease; AICore, VLLM residency, and resident processes are recorded and then evaluated through same-binary and paired-sample statistics.

Blocker observed 2026-09-23T21:49–21:54Z on cann-server3:

1. **VLLM was not stopped.** VLLM processes were owned by `root` (pids 82397, 83352 and children) and `zhangkaijie` (pid 2992910). This route ran as `lelinfeng` (uid 1075). Other users' services were left untouched.
2. **Resident VLLM was observed** across ~5 minutes of sampling (10 polls): every device showed VLLM resident; free HBM was ~1k–6.3k of 65536 pages (~91–99% used). This explains the recorded load; it is not a current compile stop condition.
3. **Concurrent next6 activity:** other routes compiling/probing (REDUCE-INVSCALE-X, UB-LIVENESS-X) on shared host; device 6 previously shared with `ub_liveness_*` probe process.

Evidence: `support/probe-blocker-r3/`.

## Preserved probe sets (do not overwrite)
- set-1 contaminated: `support/contaminated-set-1/` + server `results/probes_contaminated_1/`
- set-2 historical load-poll attempt: `support/clean-window-set-2/` + server `results/probes_clean_window_2/`

## Resume condition (Main / ops only)
Run the same four pairs when the device has live HBM utilization below 100%, the shared lease is unconflicted, and no other Route timing process is active on that device. Same-binary qualification and shape-specific statistics still decide whether P/C may start.

No ONLINE until a genuine attributable window exists or Main directs otherwise.
No CANNJudge by this agent.

## Main r3 (2026-09-23T21:59:59Z)
ACCEPT. Decision remains NEEDS_ONE_MORE_LOCAL + PROBES PARKED.
Stop VLLM polling. Use the current HBM/lease admission rule and requalify the exact shape when Main assigns the route. See MAIN-REVIEW-R3.md.
