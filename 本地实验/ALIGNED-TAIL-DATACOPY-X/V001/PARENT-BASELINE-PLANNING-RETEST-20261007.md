# ALIGNED-TAIL-DATACOPY-X Parent Baseline Retest

- Date: `2026-10-07` UTC
- Trigger: new Planning directive authorizing minimum route-local Parent build/harness verification
- Worktree / branch: `/home/data4t2/lelinfeng/cann-r-w4-5-aligned-tail` / `route/r-w4-5-aligned-tail-datacopy-x`
- Parent source: unchanged `submission.asc`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate source: not created or edited

## Compile

- Result: `PASS`
- Build directory: `build-parent-planning-20261007T2220Z/`
- Command: CMake configure with `PARENT_SOURCE` set to this exact `submission.asc`, followed by `cmake --build ... --target parent_runner --parallel 1`
- Raw log: `parent-compile-planning-20261007T2220Z.log`
- Parent runner SHA256: `3eecd98ee6b5cdb54b13d26134014276b2c39bab7456996f2a9b0e9f7e8482c0`
- Toolchain: CANN `ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest`; `bisheng` 15.0.5; CMake 3.22.1

The first shell attempt stopped before CMake because `set_env.sh` reads `LD_LIBRARY_PATH` while nounset was enabled. Retrying without nounset completed configure, compile, and link. No existing build directory or log was overwritten.

## Correctness

- Result: `FAIL`, process return code 1
- Runner command: `parent_runner --suite correctness --device 0`
- Device: NPU 0, 910B3 / DAV_2201; 40 vector cores, 40 launch cores
- Pre-run and post-run device snapshots: `parent-correctness-planning-20261007T2220Z-device-pre.txt` and `parent-correctness-planning-20261007T2220Z-device-post.txt`
- Full raw log: `parent-correctness-planning-20261007T2220Z.log`
- Summary: 60/66 PASS; 6/66 FAIL
- Failing cases: FP32 `(rows,width)` = `(4,8193)`, `(2,16383)`, `(2,16384)`, `(2,16385)`, `(1,32768)`, `(1,32769)`
- The pre-run snapshot showed no running process on NPU 0 and 3431/65536 MB HBM usage; the post-run snapshot showed no running process and the same HBM usage.

The same six wide-FP32 cases fail as in the three preserved full-suite Parent runs. Actual output summaries again vary despite the deterministic harness seeds and unchanged Parent SHA. This confirms the Parent baseline remains invalid in this route-local harness; it does not assign root cause between the kernel and runner. The earlier `PARENT-REPEAT-OUTPUT-DIAGNOSTIC-20261007.md` and all prior raw logs remain unchanged.

## Gate

- Parent Correctness: `FAIL`
- Parent Local: `NOT_RUN` in this retest because the correctness gate failed
- Paired Parent/Candidate timing: unavailable; `LOCAL_SCORE=NONE`
- Candidate V001: not created; no Candidate source edit
- Online: not run
- Classification: retain `PARENT_HARNESS_OR_BASELINE_BLOCKED`

No score is inferred from the prior unpaired Parent benchmark log. Resume only after Planning supplies authoritative Parent-validity evidence or a runner/ABI contract that resolves the gate.
