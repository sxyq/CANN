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

## R015C-r2 declaration (2026-09-24)

- Parent decision: `R015C-r1` is `LOCAL_REJECTED` because NPU correctness passed `(R=2,D=256)` but failed `(R=5,D=4096)` and `(R=3,D=8192)` from output index 2048.
- Direct parent: `R015C-r1`; parent source SHA-256: `5ee82b00760a770c06bd2f1a51fb11c892580458b3a43169a75010a5091b26b0`.
- New revision: `R015C-r2`; current source SHA-256 at declaration: `5ee82b00760a770c06bd2f1a51fb11c892580458b3a43169a75010a5091b26b0`.
- Correctness-only hypothesis refinement: on dav_c220, the direct 2-D GM/UB DataCopy produces wrong output once one row exceeds the observed 256 x 32-byte transfer boundary, despite legal 32-byte block units and MTE2 ordering; keep `DataCopyParams.blockLen` in /32 units, use `blockCount=1` segments no larger than that boundary, preserve `VECCALC` and the existing MTE2 ordering, add only the MTE3 ordering needed before UB reuse, and cover the odd-row tail.
- Scope: no performance intent, no timing, no online submission, and no source change had been made when this declaration was recorded.

## R015C-r2 pre-edit DMA boundary evidence (2026-09-24)

- Server3 CANN 8.5.0.alpha002 `dav_c220` headers define `DataCopyParams` as `uint16_t blockCount` and `uint16_t blockLen`; local validation accepts `blockCount` 1..4095 and `blockLen` 1..65535, and the implementation passes `blockLen * 32` bytes to the DMA primitive.
- The r1 device result first diverged at FP32 element 2048 for both D=4096 and D=8192. Those rows requested 512 and 1024 32-byte blocks respectively, while the first 256 blocks matched. The device evidence therefore identifies an observed dav_c220 2-D per-row boundary at 256 blocks even though the software field range is wider; no narrower limit was present in the inspected local header.
- The declared r2 source change is limited to one-row, at-most-256-block segments with `blockLen = chunkElements * sizeof(float) / 32`, plus `PIPE_MTE3` ordering before reusing the same UB staging tensor. `VECCALC`, the existing `PIPE_MTE2` ordering, row/tail coverage, and all host ABI files remain unchanged.
- At this declaration point the source remained unchanged: current source SHA-256 `5ee82b00760a770c06bd2f1a51fb11c892580458b3a43169a75010a5091b26b0`.

## R015C-r2 source diff before server3 build (2026-09-24)

- Current source SHA-256: `9471d7c2faf12c8d5d31e3cc3865fe2a703362b2d8cdb635beda2e64ebe259be`.
- Only `op_kernel/row_copy_kernel.asc` changed from the declared r2 source: the single 2-D copy became nested row/column segments with `blockCount=1`; each `DataCopyParams.blockLen` remains `chunkElements * sizeof(float) / 32` and is capped at 256 blocks (2048 FP32 elements); `PIPE_MTE2` remains between copy-in and copy-out, and `PIPE_MTE3` now completes copy-out before the same UB tensor is reused.
- No TBuf position, host ABI, tiling file, build script, performance setting, or online object changed.

## R015C-r2 server3 build/link and targeted NPU correctness (2026-09-24)

- Server and working directory: `cann-server3`, `/home/data4t2/lelinfeng/MODE-X-R015C_runs/dispatch_20260924`.
- Route-owned cache action: removed only `build/` and `output/` under that directory before configuring; source files and prior logs were retained. No other route or shared control path was touched.
- Build command: `source /usr/local/Ascend/ascend-toolkit/set_env.sh && bash build_and_probe.sh`; compile and link passed, and host probes passed for `(R=2,D=256)`, `(R=5,D=4096)`, and `(R=3,D=8192)`. Existing `cce_global` ignored-attribute warnings remained on host launch arguments.
- Build log: `/home/data4t2/lelinfeng/MODE-X-R015C_runs/dispatch_20260924/build_r015c_r2_20260924.log`, SHA-256 `b1e04edc98bf06591469a3f48256dbe0545d8a88dd88e5fe7d2bc8ad9b62a6d9`.
- Device commands: `timeout 60s env ASCEND_DEVICE_ID=4 ASCEND_SLOG_PRINT_TO_STDOUT=1 build/row_copy_probe --device-probe 2 256 4`; the same command with `5 4096 4`; and the same command with `3 8192 4`.
- Source SHA-256 used by the binary: `9471d7c2faf12c8d5d31e3cc3865fe2a703362b2d8cdb635beda2e64ebe259be`.
- NPU result: `FAIL` for all three cases, each status 3 and first mismatch at index 0: D=256 expected `0.741935`, actual `-0.905264`; D=4096 expected `0.741935`, actual `0.801747`; D=8192 expected `0.741935`, actual `-0.729481`.
- The correctness log contains no `507035`, vector-core exception, or MTE address-range message; this revision's result is an output mismatch from the segmented path. Stop at r2; no timing or online submission was run.
- Correctness-run log: `/home/data4t2/lelinfeng/MODE-X-R015C_runs/dispatch_20260924/correctness_r015c_r2_20260924.log`, SHA-256 `8b5073e6ed56db78b85c93582b6706edc009fd01c1be0c76d71875f97928a1a2`.
- Load during the run: device 4 had `AICore=0%`, `HBM=59188/65536 MB`, and an active `VLLMEngineCor` process using `55664 MB`; devices 0-3 and 7 had existing VLLM/python activity and high HBM occupancy. No latency result was collected; any future timing remains `LOAD_CONTAMINATED`.

## R015C-r3 declaration (2026-09-24)

- Parent decision: `R015C-r2` is `LOCAL_REJECTED` because the segmented `DataCopy` path failed at index 0 for `(R=2,D=256)`, `(R=5,D=4096)`, and `(R=3,D=8192)`.
- Direct parent: `R015C-r2`; parent and current source SHA-256 at declaration: `9471d7c2faf12c8d5d31e3cc3865fe2a703362b2d8cdb635beda2e64ebe259be`.
- New revision: `R015C-r3`.
- Same boundary hypothesis: retain the observed dav_c220 limit of 256 x 32-byte blocks per row. The final correctness-only source change will replace the r2 GM↔UB segment calls with Ext `DataCopyPad`, where `blockLen` is the exact byte length of a segment no larger than 256 blocks; both directions use explicit `local[0]`, exact aligned no-pad parameters, and full ordering before staging reuse.
- Scope: no TBuf position change, no host ABI or tiling change, no single transfer above 256 blocks, no performance target, no timing, and no online submission.

## R015C-r3 source diff before server3 build (2026-09-24)

- Current source SHA-256: `e1786bec2673519f41887fdffa0e11751f515a81037d854c88ae7edc0e4af903`.
- Only `op_kernel/row_copy_kernel.asc` changed from r2: each one-row segment now uses Ext `DataCopyPad` in both directions, with byte-length `blockLen = chunkElements * sizeof(float)`, explicit `local[0]`, `isPad=false`, zero padding, and `PIPE_ALL` after each direction. The segment cap remains 256 32-byte blocks; `VECCALC`, host ABI, tiling, and build files remain unchanged.

## R015C-r3 server3 build/link and targeted NPU correctness (2026-09-24)

- Server and working directory: `cann-server3`, `/home/data4t2/lelinfeng/MODE-X-R015C_runs/dispatch_20260924`.
- First build attempt used source SHA-256 `e502994367a55b70fce99a9a764ad6501829281b32c979779ee8f3374cb052ec` and failed at `row_copy_kernel.asc:40`: `DataCopyExtParams.blockLen` initialization narrowed `unsigned long` to `uint32_t`; no device command ran. Log: `/home/data4t2/lelinfeng/MODE-X-R015C_runs/dispatch_20260924/build_r015c_r3_20260924.log`, SHA-256 `06abbd36c00b9839fd4ac99e26597715d53359ec974856a55658da02b89f6e0f`.
- After the explicit byte-length cast, retry command was `source /usr/local/Ascend/ascend-toolkit/set_env.sh && bash build_and_probe.sh`; compile and link passed, and host probes passed for `(R=2,D=256)`, `(R=5,D=4096)`, and `(R=3,D=8192)`. Existing `cce_global` ignored-attribute warnings remained. Retry log: `/home/data4t2/lelinfeng/MODE-X-R015C_runs/dispatch_20260924/build_r015c_r3_retry_20260924.log`, SHA-256 `e7f3821b5a4b09d2fb2c84304a86546c91eff906667e091521f1e50879be4fe9`.
- Device commands: `timeout 60s env ASCEND_DEVICE_ID=4 ASCEND_SLOG_PRINT_TO_STDOUT=1 build/row_copy_probe --device-probe 2 256 4`; the same command with `5 4096 4`; and the same command with `3 8192 4`.
- Source SHA-256 used by the binary: `e1786bec2673519f41887fdffa0e11751f515a81037d854c88ae7edc0e4af903`.
- NPU result: `PASS` for all three cases, status 0 and `exact=1`: `(R=2,D=256,elements=512)`, `(R=5,D=4096,elements=20480)`, and `(R=3,D=8192,elements=24576)`. Overall status 0.
- Correctness log: `/home/data4t2/lelinfeng/MODE-X-R015C_runs/dispatch_20260924/correctness_r015c_r3_20260924.log`, SHA-256 `61b4a78fb0d23176ba8d08347341f852102b9e026aa10cb29aac7f416fd39e3e`. The log contains no `507035`, vector-core exception, MTE address-range message, or mismatch.
- Load during correctness: device 4 had `AICore=0%`, HBM `59236/65536 MB` before and `59240/65536 MB` after; `VLLMEngineCor` used `55664 MB`, with an additional small `async_triple_x_` process. Other devices retained existing VLLM/python activity and high HBM occupancy. No latency result was collected; future timing remains `LOAD_CONTAMINATED`.
- Prior decision: `R015C-r3 LOCAL_PASS`, ready for Main Review; no timing and no online submission.

## R015C-r3 paired local correctness (2026-09-24)

- The r2 parent and r3 candidate were run on the same deterministic input formula `(i*17+23)%1009/31.0`, device 4, for three paired shapes: `(R=2,D=256)`, `(R=5,D=4096)`, and `(R=3,D=8192)`.
- The r2 parent build/link and host probes passed. Its device result failed at index 0 for all three shapes. The r3 candidate passed exact comparison for all three shapes; host probes passed for both binaries.
- Paired log: `/home/data4t2/lelinfeng/MODE-X-R015C_runs/dispatch_20260924/paired_local_r015c_r3_20260924.log`, SHA-256 `9022aa637050ef12fe6b4db554313f4639b33cd46ed4607a988494966de01295`.
- The paired run had device 4 `AICore=0%`, HBM `59187/65536 MB` before and `59189/65536 MB` after, with `VLLMEngineCor` using `55664 MB`; other devices retained active VLLM/python work and high HBM occupancy. The run is `LOAD_CONTAMINATED` for any latency interpretation.
- Because the r2 parent failed correctness, retain r3 correctness evidence only. No performance comparison, timing, or online submission was run. Package decision remains `NEEDS_ONE_MORE_LOCAL`.
