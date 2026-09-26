# SCHED-ROWGROUP-X V001 — Probe park (Main review r2 → r3 attempt)

## Decision
- V001 source unchanged (SHA 0fae0a42e3942356fe3477cd518b17180b6104d57350cee705cba37a5895e65c).
- CORRECTNESS remains PASS (from prior local matrix).
- **Probes PARKED** — no set-3 run.
- Handoff: **NEEDS_ONE_MORE_LOCAL** with **PARK recommendation for probes**.

## Why no genuine VLLM-free attempt
Main required: VLLM stopped OR HBM free enough to attribute paired magnitudes.

Blocker observed 2026-09-23T21:49–21:54Z on cann-server3:

1. **Cannot stop VLLM.** VLLM processes are owned by `root` (pids 82397, 83352 and children) and `zhangkaijie` (pid 2992910). This route runs as `lelinfeng` (uid 1075). Stopping other users' long-running services is out of scope for this route agent.
2. **No VLLM-free window** across ~5 minutes of sampling (10 polls): every device showed VLLM resident; free HBM only ~1k–6.3k of 65536 pages (~91–99% used).
3. **Concurrent next6 activity:** other routes compiling/probing (REDUCE-INVSCALE-X, UB-LIVENESS-X) on shared host; device 6 previously shared with `ub_liveness_*` probe process.

Evidence: `support/probe-blocker-r3/`.

## Preserved probe sets (do not overwrite)
- set-1 contaminated: `support/contaminated-set-1/` + server `results/probes_contaminated_1/`
- set-2 clean-window attempt: `support/clean-window-set-2/` + server `results/probes_clean_window_2/`

## Resume condition (Main / ops only)
Run same four pairs only when:
- VLLM fully stopped on probe device **or** free HBM high enough that aligned-D control is near zero, **and**
- no concurrent next6 probe on that device.

No ONLINE until a genuine attributable window exists or Main directs otherwise.
No CANNJudge by this agent.

## Main r3 (2026-09-23T21:59:59Z)
ACCEPT. Decision remains NEEDS_ONE_MORE_LOCAL + PROBES PARKED.
Stop VLLM polling. Wait for Main ops clean window. See MAIN-REVIEW-R3.md.
