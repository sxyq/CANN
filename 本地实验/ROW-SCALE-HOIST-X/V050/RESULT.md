# ROW-SCALE-HOIST-X V050 Result

- Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`).
- Change: in FP32 `ProcessSmallFp32Batched`, apply each row's `invRms` to reusable FP32 gamma scratch before multiplying the retained value row.
- Compile: PASS for `device` and `submission`; see `compile-result.json` and `compile.log`.
- Device-0 pre-correctness snapshot at `2026-10-08T04:34:17Z`: HBM `9137/65536 MB` used (`56399 MB` free), exceeding the assigned `100 MB` threshold. Existing process/AICore activity was observed and left undisturbed.
- Correctness: Parent PASS, matched ratio `1.0`, maximum absolute error `3.09944153e-06`. Candidate FAIL, matched ratio `0.996419271`, maximum absolute error `1.47696984` against `0.01`. Both use FP32 `[128,3072]`, 40 blocks, 3-4 rows/core; dispatch audit selects `ProcessSmallFp32Batched`. Candidate audit confirms the changed source executed.
- Invocation handling: both binaries entered the runtime and completed. Parent process exit was `0`, Candidate process exit was `2`; only the outer pipeline exited `1` because tee was given a misspelled Japanese directory. Complete stdout, exit codes, intended/actual paths, and recovery details are retained in `correctness-parent.log`, `correctness-candidate.log`, `correctness-status.txt`, and `correctness-invocation.txt`. No runner was relaunched to repair logging.
- Local: NOT RUN because Candidate correctness failed.
- Device 0 was explicitly released at `2026-10-08T04:41:56Z` after evidence capture; see `device-release.log`.
- Verdict: `CORRECTNESS_FAILED`. Current Local Best remains V026. Online was not run.
