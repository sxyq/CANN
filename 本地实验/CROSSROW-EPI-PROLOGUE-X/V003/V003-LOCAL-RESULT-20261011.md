# W5-R01 CROSSROW-EPI-PROLOGUE-X V003 shape-level Local result

AGENT_ID: W5-R01/Zeno
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V003
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
SOURCE_COMMIT: `5347a5606cdc8aec4c350c7d6e0b987b94346a8c`
LOCAL_HOST: `hwnput3`
LOCAL_DEVICE: `NPU2`

## Source, artifact, and runner identity

PARENT_REVISION: V002 Candidate (V003 Parent snapshot)
PARENT_SOURCE_SHA256: `426489bfdc63441f2fd179fdff176875b24d815e6e0420343aeed0b0dab7933f`
CANDIDATE_SOURCE_SHA256: `c6d8da88a9ba03e6c730733c52bed051b54ebc94629196636ea415599b96eb8c`
PARENT_SO_SHA256: `291eabc271043752cc48466aea0d3ec994476cd3340a4e97f3143efe69dcb7bc`
CANDIDATE_SO_SHA256: `502006729d37f9ab654ab460efef9aa4318bb7fa9cb107c066e0e0ad6b1bf342`
RUNNER_PATH: `本地实验/CROSSROW-EPI-PROLOGUE-X/V001/support/build/crossrow_v001_paired_runner`
RUNNER_SHA256: `5210262b45c50878c24def38e108386b45bd017086a61a1293ab290d171a4d5a`
RUNNER_SOURCE_SHA256: `e4c46069d21f16b863775461298b452b063430d7ca14fdad3c56ae8c0826d938`
RUNNER_ABI_SHA256: `0897a04ba82bda39d53e77d548966eb54a6b7ec16c17387dccf26d86cbf577d2`

The V001 adapter include paths were temporarily pointed at V003 Parent/Candidate and the existing paired-runner CMake project was clean-built. The resulting Parent/Candidate/Runner hashes match the V003 Build/Correctness receipt. No Kernel, ABI, Runner, or V002 file was edited. After Local, both adapters were restored to V001 paths and the existing CMake project was clean-built again; tracked adapter diff is empty.

TEMP_SELECTOR_DIFF:

```diff
- #include "../Parent.asc"
+ #include "../../V003/Parent.asc"
- #include "../Candidate.asc"
+ #include "../../V003/Candidate.asc"
```

RESTORED_PARENT_ADAPTER_SHA256: `2013d0e9e188501fe953dcf7bbd55e0f9b800e71b9e54ce29d6ce3152eaddc28`
RESTORED_CANDIDATE_ADAPTER_SHA256: `a7dd3c814fc752cf94bc9d98b918983acc8e73fc66240d6b6599d6057e1e8932`
RESTORE_BUILD_RC: `0`
RESTORED_PARENT_SO_SHA256: `61eb3391924fabbc7b72231652914df2688ca28d376ed143b5d351042b93e944`
RESTORED_CANDIDATE_SO_SHA256: `278dbd1cd0866f30ce25b2ea9a3c5fd5214b641bc8ae78e3de52b6d35c3ff34b`
RESTORED_RUNNER_SHA256: `5210262b45c50878c24def38e108386b45bd017086a61a1293ab290d171a4d5a`

## Shape and execution

SHAPE: `rows=16,width=6144,blocks=8,dtype=fp16`
COMMAND: `crossrow_v001_paired_runner --mode local --device 2 --rows 16 --width 6144 --blocks 8 --dtype fp16 --warmups 45 --repeats 21 --output local-v003-r16-d6144-w45-npu2-20261011.tsv`
LOCAL_RC: `0`
WARMUPS: `45`
PAIRED_SAMPLES: `21/21`, IDs `0..20`
ORDER: alternating `PC/CP`
TIMING: device events through the existing Runner's event-record/elapsed-time path
SAME_BINARY_PROCESS: `YES`; both identity-verified shared libraries were loaded by the same paired-runner process
PARENT_ONLY_SAME_BINARY_QUALIFICATION: `NOT_RUN`; the existing CLI has no separate Parent-only qualification mode
INLINE_RUNNER_CORRECTNESS: `PASS`, zero bit differences; this was the existing Local mode's built-in check, not a separate correctness invocation

The raw TSV header prints the Runner's historical V001/W3 metadata labels. Those labels were not used as source identity; the source and both loaded library hashes above were verified immediately before the invocation. The raw file is retained unchanged.

## Device, lease, and load quality

PRE_SNAPSHOT: `local-v003-r16-d6144-w45-npu2-20261011-pre.txt`, `2026-10-11T04:55:29Z`
POST_SNAPSHOT: `local-v003-r16-d6144-w45-npu2-20261011-post.txt`, `2026-10-11T04:56:59Z`
NPU2_HBM_PRE_POST: `63350/65536 MB used`; approximately `2186 MB free` in both snapshots
NPU2_AICORE_PRE_POST: `55% / 58%`
NPU2_PROCESS_PRE_POST: PID `658669`, `python`, `59977 MB` in both snapshots
LEASE_CONTEXT: per current user-provided coordination snapshot, only NPU6 had an unended shared lease; NPU2 was selected. Under the current published rule as relayed by the user, lease metadata is recorded and is not a Local authorization gate.
OTHER_DEVICE_LOAD: present and recorded in both full snapshots; it did not gate execution.
LOAD_QUALITY: `TARGET_EXTERNAL_PROCESS_PERSISTENT / HIGH_JITTER`; exact per-sample process overlap was not instrumented.

The earlier `V003-LOCAL-DIAGNOSTIC-LEASE-BLOCKER-20261010.md` and `local-preflight-identity-20261011.log` are preserved as historical preflight evidence. The lease interpretation in the former is superseded by the current published-rule clarification. The artifact mismatch in the latter was resolved by the V003-bound clean build recorded here.

## Raw and statistics

RAW_PATH: `本地实验/CROSSROW-EPI-PROLOGUE-X/V003/local-v003-r16-d6144-w45-npu2-20261011.tsv`
RAW_SHA256: `1bf555ca8e3cfc3e7af96046c55434afd13be0b0474027eedad0dc899b39423c`
RUN_LOG: `local-v003-r16-d6144-w45-npu2-20261011.log`
RUN_LOG_SHA256: `613af7e530c85d4877cf397c350689eaf882bf9c3f0b73d709054de84885b218`

PARENT_MEDIAN_DEVICE_US: `12.500000`
CANDIDATE_MEDIAN_DEVICE_US: `19.140000`
MEDIAN_RATIO_FORMULA: `100 * (median(C) - median(P)) / median(P)`
MEDIAN_RATIO: `+53.120000%` (Candidate slower)
MEDIAN_PAIRED_DELTA_US: `+6.860000`
MEDIAN_OF_PAIRED_PERCENT_FORMULA: `median_i(100 * (C_i - P_i) / P_i)`
MEDIAN_OF_PAIRED_PERCENT: `+50.137739%`
PAIR_DIRECTION: Candidate faster `3/21`, slower `18/21`, ties `0`
PARENT_JITTER: mean `13.725714 us`, sample SD `5.186456 us`, CV `37.7864%`, min/max `7.860000/25.140001 us`
CANDIDATE_JITTER: mean `18.574286 us`, sample SD `6.114308 us`, CV `32.9181%`, min/max `8.220000/32.699998 us`

## Verdict

LOCAL_STATUS: `NEEDS_VALIDATION / CONTAMINATED`
LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL`
NO_PROMOTION: `YES`
LOCAL_BEST: `UNCHANGED`
LOCAL_SCOPE: `SHAPE_LOCAL_ONLY`
FULL_LOCAL_SCORE: `NOT_COMPUTABLE`
COMPLETE: `NO`
V002_RERUN: `NO`

The single diagnostic shows a large Candidate-slower signal, but target-device external activity and broad sample spread make this shape-level result insufficient to claim a stable causal regression or promotion. Keep all 21 pairs; do not filter or rerun this contrast as part of this receipt.

## NPU7 follow-up diagnostic — 2026-10-11 06:29 UTC

This is an additional V003 shape-level diagnostic on the coordinator-assigned NPU7. It does not create V004, change V003 source, rerun V001/V002, or alter any prior raw.

DEVICE: `NPU7`
SHAPE: `rows=16,width=6144,blocks=8,dtype=fp16`
PRE_SNAPSHOT: `local-v003-r16-d6144-w45-npu7-20261011-pre.txt`, `2026-10-11T06:29:02Z`
POST_SNAPSHOT: `local-v003-r16-d6144-w45-npu7-20261011-post.txt`, `2026-10-11T06:29:54Z`
NPU7_HBM_PRE_POST: `28636/65536` and `28635/65536 MB used`; approximately `36900 MB` free
NPU7_AICORE_PRE_POST: `3% / 6%`
NPU7_PROCESS_PRE_POST: PID `439848` (`python3`, `21260 MB`) and PID `445986` (`python`, `3898 MB`) present in both snapshots
LOAD_QUALITY: `TARGET_EXTERNAL_PROCESS_PERSISTENT / HIGH_JITTER`
LEASE_CONTEXT: NPU7 was assigned to R1 by the current coordination update; lease metadata was recorded and did not gate execution.

V003_BUILD_FOR_NPU7_COMMAND: `source /usr/local/Ascend/ascend-toolkit/set_env.sh; export CPLUS_INCLUDE_PATH and CPATH to the HCC C++ root/target/backward; cmake --build 本地实验/CROSSROW-EPI-PROLOGUE-X/V001/support/build --clean-first --parallel 1`
V003_BUILD_FOR_NPU7_RC: `0`
V003_BUILD_FOR_NPU7_LOG: `build-v003-npu7-20261011.log`
V003_PARENT_SO_SHA256: `291eabc271043752cc48466aea0d3ec994476cd3340a4e97f3143efe69dcb7bc`
V003_CANDIDATE_SO_SHA256: `502006729d37f9ab654ab460efef9aa4318bb7fa9cb107c066e0e0ad6b1bf342`
V003_RUNNER_SHA256: `5210262b45c50878c24def38e108386b45bd017086a61a1293ab290d171a4d5a`

LOCAL_COMMAND_NPU7: `crossrow_v001_paired_runner --mode local --device 7 --rows 16 --width 6144 --blocks 8 --dtype fp16 --warmups 45 --repeats 21 --output local-v003-r16-d6144-w45-npu7-20261011.tsv`
LOCAL_RC_NPU7: `0`
RAW_NPU7: `local-v003-r16-d6144-w45-npu7-20261011.tsv`
RAW_NPU7_SHA256: `fca6026ff65d3e955c453c6513fd971b150e9f225fdfd76b30755aabe1fb8e27`
RUN_LOG_NPU7_SHA256: `83a7926732e4ca95503a49285fe937dd892574e97ff9e10b45c6a1d8cee674eb`
SAMPLES_NPU7: `21/21`, IDs `0..20`, alternating `PC/CP`, device-event timing
PARENT_ONLY_SAME_BINARY_QUALIFICATION: `NOT_RUN`; existing Runner has no separate Parent-only mode

PARENT_MEDIAN_NPU7_US: `18.279999`
CANDIDATE_MEDIAN_NPU7_US: `17.060000`
MEDIAN_RATIO_NPU7_FORMULA: `100 * (median(C) - median(P)) / median(P)`
MEDIAN_RATIO_NPU7: `-6.673956%`
MEDIAN_PAIRED_DELTA_NPU7_US: `-0.280000`
MEDIAN_OF_PAIRED_PERCENT_NPU7_FORMULA: `median_i(100 * (C_i - P_i) / P_i)`
MEDIAN_OF_PAIRED_PERCENT_NPU7: `-1.869159%`
PAIR_DIRECTION_NPU7: Candidate faster `11/21`, slower `10/21`, ties `0`
PARENT_JITTER_NPU7: mean `25.637143 us`, sample SD `41.085355 us`, CV `160.2572%`, min/max `6.960000/204.019994 us`
CANDIDATE_JITTER_NPU7: mean `16.812381 us`, sample SD `4.080958 us`, CV `24.2735%`, min/max `9.220000/25.200000 us`
NPU7_OUTLIER: Parent sample includes `204.019994 us`; retained in raw and all statistics.

LOCAL_STATUS_NPU7: `NEEDS_VALIDATION / CONTAMINATED`
LOCAL_VERDICT_NPU7: `NEEDS_ONE_MORE_LOCAL`
NO_PROMOTION_NPU7: `YES`
LOCAL_BEST_AFTER_NPU7: `UNCHANGED`
LOCAL_SCOPE_NPU7: `SHAPE_LOCAL_ONLY`
FULL_LOCAL_SCORE_NPU7: `NOT_COMPUTABLE`

## Route counters at this delivery

COUNTING_BASIS: `TOTAL_ATTEMPTS` counts route-declared Kernel Revision attempts, not repeated raw runs; Build/Correctness counts revisions with a verified PASS; no full-suite score is inferred.
TOTAL_ATTEMPTS: `3`
KERNEL_REVISIONS_DECLARED: `3` (`V001`, `V002`, `V003`)
BUILD_PASS: `3` revision-level PASS (`V001`, `V002`, `V003`)
CORRECTNESS_PASS: `3` revision-level PASS (`V001`, `V002`, `V003`)
FULL_LOCAL_SCORED_REVISIONS: `0`
LOCAL_BEST_IMPROVEMENTS: `0`
LATEST_REVISION_SCORE: `V003 / SHAPE_LOCAL_ONLY; latest NPU7 median(C)=17.060000 us, median-ratio=-6.673956%; FULL_LOCAL_SCORE=NONE`
BEST_LOCAL_SCORE: `SHAPE_LOCAL_ONLY / NONE`
CURRENT_GIT_SHA_AT_APPEND: `9b2c904e416643d6c1f0ac3a8233826595e813b8`

RESTORE_AFTER_NPU7: V001 adapters restored to `../Parent.asc` and `../Candidate.asc`; restore clean build RC `0`; restored Parent/Candidate/Runner artifacts were `61eb3391924fabbc7b72231652914df2688ca28d376ed143b5d351042b93e944` / `278dbd1cd0866f30ce25b2ea9a3c5fd5214b641bc8ae78e3de52b6d35c3ff34b` / `5210262b45c50878c24def38e108386b45bd017086a61a1293ab290d171a4d5a`; tracked adapter diff is empty.
