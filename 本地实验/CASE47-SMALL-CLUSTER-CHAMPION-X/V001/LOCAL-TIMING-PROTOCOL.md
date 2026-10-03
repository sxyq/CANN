# CASE47-SMALL-CLUSTER-CHAMPION-X V001 Local Timing Protocol

Status: registered before timing; no timing run is authorized by this file.

## Identity and scope

- Candidate: `submission.asc`, source SHA-256 `be313f80e5b088f1fe907f2f4221c61ab59ef720ce68cc9f9c941be20239c2db`.
- Direct Parent: `线上结果/R31B/V011/submission.asc`, source SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- One runner executable dynamically loads either the Parent or Candidate module. Both modules use the same arguments, input generator, allocations, warmup, event boundaries, and output path behavior.
- Official case4/case7 shape, dtype, and dispatch mapping: UNKNOWN. Every row in `support/timing-matrix.tsv` is synthetic `PROXY` input.
- Candidate correctness and exact-source Build already passed as recorded in `local-result.json`; Local timing remains NOT_RUN.

## Registered shape matrix and order

Run the stages for each row in matrix order. A row reaches Parent window qualification only after same-binary PASS, and reaches P/C only after both Parent windows pass. Finish the Parent stages for all rows before starting any P/C pair. Keep every result, including failed qualification and runner errors.

| Order | Shape | Decision role | Input | Rows | Width | Dtype |
|---:|---|---|---|---:|---:|---|
| 1 | `PROXY_D257_FP32_M2A` | Primary H1 hypothesis | PROXY | `2*A` | 257 | FP32 |
| 2 | `PROXY_D257_FP16_M2A` | Dtype guard | PROXY | `2*A` | 257 | FP16 |
| 3 | `PROXY_D257_BF16_M2A` | Dtype guard | PROXY | `2*A` | 257 | BF16 |
| 4 | `PROXY_D257_FP32_MA_FALLBACK` | `localRows == 1` fallback guard | PROXY | `A` | 257 | FP32 |
| 5 | `PROXY_D256_FP32_M2A_ALIGNED_CONTROL` | Aligned-path control | PROXY | `2*A` | 256 | FP32 |

`A` is read on the selected device at run time using `ACL_DEV_ATTR_VECTOR_CORE_NUM`. The five shapes are fixed; no post-run shape selection or Official-case claim is allowed.

## Measurement sequence

1. Main supplies an active lease for the selected device. Record the lease identifier, device state, HBM, AICore, process inventory, toolkit, source/module/runner identities, and disk state. This Route does not acquire a lease.
2. Stage 1 is Direct Parent same-binary on the exact shape: one fresh process, 45 launches each followed by stream synchronization, then two 21-sample event blocks separated by 30 seconds in that same process. Buffers and stream stay alive across both blocks. D2H follows the second block. Continue to Stage 2 for this row only if combined MAD/median `<= 0.10` and absolute block-median drift divided by the combined median `<= 0.10`.
3. Stage 2 is a separate Parent-only window qualification. After Stage 1 passes, run PRECHECK-A as six fresh processes, each with 45 synchronized warmups and one 21-sample event block. Wait 30 seconds, then run PRECHECK-B as six more fresh processes with the same protocol. For each window, compute CV and max/min from its six per-process device-event medians; each window must have CV `<= 0.15` and max/min `<= 1.30`. Run PRECHECK-B even if PRECHECK-A misses a threshold, preserving the fixed record. A row reaches P/C only if both windows pass.
4. After all five rows finish Parent stages, pair only rows that passed Stage 1 and both Stage 2 windows. Each P/C arm is a fresh runner process with the same executable, 45 synchronized warmups, then 21 event samples in one block. The fixed four-pair arm order is `P,C`; `C,P`; `P,C`; `C,P`. Save `npu-smi` before and after each pair.
5. Keep every raw event and wall sample. Device-event duration is primary; steady-clock launch-plus-sync duration is secondary. Copy output to host only after each process's measured block(s). No batch replay, trimming, outlier removal, per-result order change, or correctness copy is inserted into a measured block.

## Qualification, statistics, and local decision

- Stage 1 same-binary qualification uses the one-process two-block Parent data and requires combined MAD/median `<= 0.10` plus absolute block-median drift/combined median `<= 0.10`.
- Stage 2 PRECHECK-A and PRECHECK-B are separate six-process Parent blocks. Each block's CV and max/min are computed from the six per-process device-event medians and must be `<= 0.15` and `<= 1.30`, respectively. If Stage 1 or either window does not qualify, do not run P/C for that shape; retain its evidence and continue the pre-registered matrix.
- Per block and over all raw samples, report median, mean, population stdev, CV, MAD, p10, p90, min, max, max/min, and max-minus-min. Do not remove samples.
- For each pair, delta is `(Candidate median - Parent median) / Parent median * 100`; negative favors Candidate. Report all four deltas and their median. The per-shape noise reference is the Direct Parent Stage 1 combined MAD/median.
- A stable per-shape improvement requires at least 3/4 negative pair deltas and a negative median delta whose magnitude exceeds that shape's Parent MAD/median. A stable per-shape regression uses the symmetric positive rule.
- `LOCAL_ACCEPTED` requires complete qualified data for all five rows, stable improvement on the primary FP32 shape, and no stable regression on any guard/control. `LOCAL_REJECTED` requires a stable regression on any fully measured row. Otherwise report `NEEDS_ONE_MORE_LOCAL`; an unqualified row is not treated as zero gain or as a Candidate failure.
- These are Local PROXY results only. They do not establish Official score or case4/case7 input mapping.

## Prepared server commands

The following commands are for a later session after Main issues a device lease. They do not reserve a device. Example device `7` is provisional and must be re-read at that time.

```bash
cd /home/data4t2/lelinfeng/cann/server_runs/CASE47-SMALL-CLUSTER-CHAMPION-X/V001/timing-prep-20261003
source /usr/local/Ascend/ascend-toolkit/latest/aarch64-linux/bin/setenv.bash
export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest
export LD_LIBRARY_PATH="$ASCEND_HOME_PATH/lib64:$ASCEND_HOME_PATH/aarch64-linux/lib64:${LD_LIBRARY_PATH:-}"
CASE47_RUN_ID=CASE47-V001-D7-20261003TXXXXXXZ
CASE47_LEASE_ID=MAIN_LEASE_ID_FROM_ACTIVE_LEASE
bash support/run_registered_matrix.sh 7 "$CASE47_LEASE_ID" "$CASE47_RUN_ID"
```

At preparation time, configure and build one Candidate module plus the shared runner, then the Parent module:

```bash
cmake -S support -B build-candidate -DCMAKE_BUILD_TYPE=Release \
  -DASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest \
  -DCMAKE_PREFIX_PATH=/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux/tikcpp/ascendc_kernel_cmake \
  -DSERVER_AUDIT_ROOT=/home/data4t2/lelinfeng/server_full_audit \
  -DCASE47_SOURCE_FILE="$PWD/candidate/submission.asc" \
  -DCASE47_KERNEL_TARGET=case47_candidate_kernel
cmake --build build-candidate --parallel 2 --target case47_candidate_kernel case47_timing_runner
cmake -S support -B build-parent -DCMAKE_BUILD_TYPE=Release \
  -DASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest \
  -DCMAKE_PREFIX_PATH=/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux/tikcpp/ascendc_kernel_cmake \
  -DSERVER_AUDIT_ROOT=/home/data4t2/lelinfeng/server_full_audit \
  -DCASE47_SOURCE_FILE="$PWD/parent/submission.asc" \
  -DCASE47_KERNEL_TARGET=case47_parent_kernel
cmake --build build-parent --parallel 2 --target case47_parent_kernel
sha256sum candidate/submission.asc parent/submission.asc \
  build-candidate/libcase47_candidate_kernel.so \
  build-parent/libcase47_parent_kernel.so \
  build-candidate/case47_timing_runner
```
