# N01 V021 Handoff

V021 source commit `c50d6f4a7f466fbde2c2b92eb544082cf3a1a0b5`, source SHA256 `d4f9994b9eb43e75b454143cbdb3f2b8a2222545d6ad4496fc6022c1682e5134`. The source change is in FP32 `ProcessWideFp32FullCacheRows`: the squared tile is written to `residualLocal`, then `xLocal` is reused as the `ReduceSum` scratch tile.

- CANN 8.5 configure/build and device-code extraction: PASS, all exit codes `0`; AICore binary differs from V020.
- Correctness: PASS, 4/4 on d5, Runner exit `0`.
- Paired Local: completed, 4 x 21 on d5, Runner exit `0`; severe outliers in three cases, so `LOCAL_SCORE=NONE`.
- Those Correctness/Local cases are FP16/BF16 and do not execute the changed FP32 function.
- After `/home/data4t2` recovered to about 6.8 GB free, the same `paired_runner_v016` target was rebuilt with the existing FP32 extension and system C++ include paths; build exit `0`, binary SHA256 `88025ed8e024a4b223f289799a038d82b1837daf26bc97bffc6a337f06436730`.
- FP32 Correctness ran on d5 for `[2,12288]`. The Runner launched both kernels, then stopped while checking the V011 Parent against its CPU reference: 15,389 failures, max absolute error `0.092312813`, max relative error `29.889896`, exit `3`. The Candidate comparison was not reached, so this is not a V021 correctness verdict. FP32 Local was not run.
- One-time static diagnosis found the FP32 dtype code (`0`), `[2,12288]` data shape, `[12288]` parameter shape, epsilon `1e-5`, and core request consistent with the V011 source ABI and dispatch. CMake cache and compiler dependencies identify the expected V004 Parent and V021 Candidate source paths; their recorded SHA256 values match. Parent and Candidate outputs use separate live device allocations. The CPU formula matches the kernel operation order, but the reference accumulates in `float`; the custom `1e-4` tolerance has no V011-specific FP32 test configuration in the available shared Runner. The raw log has no output values, so the Parent mismatch cause remains unresolved. Treat this as an invalid measurement chain before Candidate comparison; do not judge V021 from it.
- Runner Candidate labels `V001`/`V016` are generic slots; actual Candidate is V021. Parent is fixed R31B V011.
- d5 before FP32 build and Correctness showed HBM `91%`, AICore `0%`; existing VLLMWorker PID `3355594` remained at `56354 MB`. It was observed only and not changed. The Runner was absent before execution and exited after the failed Parent check.
- Exact input identities, distributions, and raw evidence are in `RESULTS.md` and `results/`.

## FP32 Correctness Adapter Follow-Up

- On 2026-10-13, only the existing V021 Runner adapter was changed. `CheckOutput` now evaluates both outputs, reports up to eight mismatches and the complete failure count for each, and returns a combined status. The FP32 CPU reference accumulates `y*y` and computes `invRms` in double. Tolerances and Candidate kernel source are unchanged.
- The existing `paired_runner_v016` target was rebuilt in the existing V021 build directory. Build exit `0`; Runner source SHA256 `f077541d73c4819d8a8299c85fd489b70c26a44373e396880d1f1664ac9f979e`; executable SHA256 `b3bd4a4b852647893f74911b9ac6fa0fa00261172fbef9f0246879111675d9ab`. Parent SHA256 remains `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; Candidate SHA256 remains `d4f9994b9eb43e75b454143cbdb3f2b8a2222545d6ad4496fc6022c1682e5134`.
- One `--fp32-correctness` run on d5 for `[2,12288]` reached both comparisons. V011 Parent: FAIL, 15,144 elements, `max_abs=0.092312217`, `max_rel=111.43422`. V021 Candidate: FAIL, 16,698 elements, same `max_abs=0.092312217`, `max_rel=111.43422`. Runner exit `3`. The first eight mismatches for each are in the raw log.
- The run shows a shared Parent/reference discrepancy and a Candidate/reference discrepancy; the cause remains unresolved. This does not support a V021 kernel verdict. FP32 Local was not run. Completed FP16/BF16 results were not repeated or changed.
- The NPU5 pre-run snapshot showed AICore `0%`, HBM `59876/65536 MiB`, and VLLMWorker_TP PID `3355594` using `56354 MB`. The post-run snapshot shows the same NPU5 load and process. That process was observed only.
- Current build, source identity, device snapshots, command records, and raw Correctness output are in `results/fp32-adapter-*attempt-04*`, `results/fp32-adapter-*attempt-02*`, and `results/fp32-correctness-v021-attempt-02.*`.

NEXT_ACTION: retain the FP32 measurement as unresolved and do not run paired Local. Keep the attempt-01 failure log and completed low-precision evidence unchanged; do not start V022 yet.
