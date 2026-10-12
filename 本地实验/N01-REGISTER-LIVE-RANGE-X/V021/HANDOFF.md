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

NEXT_ACTION: keep the FP32 measurement chain marked invalid before Candidate comparison. Do not repeat the unchanged Correctness command or run paired Local until a concrete Parent/reference diagnosis is available. Keep completed low-precision evidence unchanged and do not start V022 yet.
