# MODE-X-R015C load quality

## Dispatch observation

- Recorded at: 2026-09-24T00:07:11+08:00 (Asia/Shanghai; server3 clock)
- Source: Main dispatch report
- Exact observation: all 8 NPUs have existing VLLM/python processes; AICore ~31-49% on devices 0-3 and 7, 0% on 4-6, HBM heavily occupied on all devices.
- Probe interpretation: any latency probe run under this load is `LOAD_CONTAMINATED`. Small latency deltas must not be interpreted as improvement.

## Server3 snapshot

- Captured at: 2026-09-24T00:07:11+08:00 (Asia/Shanghai)
- Command: `npu-smi info`
- NPU AICore utilization: device 0 31%, 1 32%, 2 32%, 3 31%, 4 0%, 5 0%, 6 0%, 7 0%.
- HBM usage: device 0 59957/65536 MB, 1 60007/65536 MB, 2 59949/65536 MB, 3 59951/65536 MB, 4 59182/65536 MB, 5 59876/65536 MB, 6 59875/65536 MB, 7 65529/65536 MB.
- Processes: devices 0-3 and 5-6 `VLLMWorker_TP`; device 4 `VLLMEngineCor`; device 7 `python`.
- Device correctness probe selection: device 4, which had 0% AICore in this snapshot and sufficient reported free HBM for the small correctness cases. The active process remains a contention risk.

## Probe records

- No latency probes have been run yet. Any future paired local latency result must be labeled `LOAD_CONTAMINATED`.
- Correctness attempt at 2026-09-24T01:09:41+08:00 to 2026-09-24T01:10:07+08:00, server3 device 4: command `cd /home/data4t2/lelinfeng/MODE-X-R015C_runs/dispatch_20260924 && ASCEND_DEVICE_ID=4 bash build_and_probe.sh --device-probes`. Build/link and all host probes passed; `(rows=2,D=256)` passed exact NPU comparison; `(rows=5,D=4096)` failed at stream synchronization with `507035`.
- Device 4 immediately before/after that attempt: HBM usage 90%, AICore 0%, AIVector 0%; after the attempt HBM remained 90%, AICore 0%, AIVector 0%. This is an observed loaded device, but the run reached the kernel and the runtime log reported MTE write-address out-of-range on blocks 0, 1, and 2. The correctness failure is therefore recorded as a kernel address fault, not attributed to contention.
- Runtime diagnostic run at 2026-09-24T01:03:21+08:00 on `(rows=5,D=4096)` with `ASCEND_SLOG_PRINT_TO_STDOUT=1` reported `ACL_ERROR_RT_VECTOR_CORE_EXCEPTION (507035)` and `The write address of the MTE instruction is out of range`; per-block reports covered blocks 0-2.

## R015C-r1 Main Review build (2026-09-24)

- Source: `phase4/workspaces/MODE-X-R015C/op_kernel/row_copy_kernel.asc`, SHA-256 `5ee82b00760a770c06bd2f1a51fb11c892580458b3a43169a75010a5091b26b0`. The source diff against the parent candidate contains only the DataCopyParams 32-byte block-unit correction and `PipeBarrier<PIPE_MTE2>` between copy-in and copy-out; TBuf remains `VECCALC`.
- Server: `cann-server3`; working directory `/home/data4t2/lelinfeng/MODE-X-R015C_runs/dispatch_20260924`.
- Command: `bash build_and_probe.sh` (build, link, and host probes only; no device probe). Exit status 0.
- Results: `row_copy_probe` compiled and linked; host probes passed for `(R=2,D=256)`, `(R=5,D=4096)`, and `(R=3,D=8192)`. Compiler emitted existing `cce_global` ignored-attribute warnings from host launch arguments.
- Log: `/home/data4t2/lelinfeng/MODE-X-R015C_runs/dispatch_20260924/build_r015c_r1_main_review_20260924.log`, SHA-256 `cf988436014cab8f4b28cc3eedc91ba61c24e36635412122c5f492c7206f09dc`.
- NPU correctness and paired timing probes were not run; awaiting Main Review.

## R015C-r1 targeted NPU correctness (2026-09-24)

- Cache scope: removed only `/home/data4t2/lelinfeng/MODE-X-R015C_runs/dispatch_20260924/build` before the run. The Route had no `output/` directory. Source and prior logs were retained.
- Server and device: `cann-server3`, device 4. Source SHA-256: `5ee82b00760a770c06bd2f1a51fb11c892580458b3a43169a75010a5091b26b0`.
- Command sequence: source `/usr/local/Ascend/ascend-toolkit/set_env.sh`; run `bash build_and_probe.sh`; then run `build/row_copy_probe --device-probe` with `(2,256)`, `(5,4096)`, and `(3,8192)` under `ASCEND_DEVICE_ID=4 ASCEND_SLOG_PRINT_TO_STDOUT=1`, each with a 60-second timeout.
- Build and host status: PASS. Host probes for all three shapes passed.
- NPU status: `(R=2,D=256)` PASS, exact comparison; `(R=5,D=4096)` FAIL with status 3, first mismatch at index 2048 (`expected=17.1935`, `actual=0.00184807`); `(R=3,D=8192)` FAIL with status 3 and the same first mismatch. These cover FP32, minimum width, mid width, maximum width, and odd-row tail blocks.
- Runtime evidence: the run log contains no new `507035`, vector-core exception, or MTE address-range message. The failure is an output correctness mismatch after synchronization, not a timing result.
- Source selection: `op_host/row_copy_host.asc` includes `../op_kernel/row_copy_kernel.asc`, so the tested source is the `VECCALC` file above. A separate older root-level `row_copy_kernel.asc` remains at SHA-256 `1e271da22a11081e8606c19e715e8d8d87197e08b903b48f69c69dc0593d27e8` and was not compiled; it was retained.
- Load: device 4 remained at 0% AICore with `VLLMEngineCor` active and about 59.2 GB of 64 GB HBM occupied before and after the run. Mark future timing as `LOAD_CONTAMINATED`; no timing was run here.
- Log: `/home/data4t2/lelinfeng/MODE-X-R015C_runs/dispatch_20260924/correctness_r015c_r1_20260924.log`, SHA-256 `70e6a064d449f8fbe1d6531f3a9bc0f5d1726bc30bb260c440df089d6aec3540`.
- The outer shell recorder printed a literal `PIPESTATUS` expression and returned 2 after all cases completed; per-case statuses and `OVERALL_STATUS=3` in the log are the authoritative result.
- No source, declaration, performance setting, or online submission was changed after Main Review.
