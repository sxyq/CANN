# N04 V014 Handoff

V014 is a single-factor residual-stream hint trial based on N04 V013. It changes only the wide low-precision `residualGm_` hint from `CACHE_MODE_NORMAL` to `CACHE_MODE_PERSISTENT`; `xGm_` remains `CACHE_MODE_DISABLE` and output remains `CACHE_MODE_DISABLE`. Compute and dataflow behavior are unchanged.

Using the existing server3 CMake entry and `paired_runner_v016`, Compile passed, Champion/Candidate Correctness passed 4/4 on d7, and paired Local completed 4x21 with Runner return code 0. Overall `LOCAL_SCORE=NONE`: both FP16 cases have large two-sided outliers; BF16 tail and wide each retain a compact 21/21-negative per-case signal. All raw samples and snapshots remain in this result directory. Local evidence is one run and does not represent Official results.

Full source identity and per-case statistics are in `RESULTS.md`. Next, continue with a distinct GM/L2 hint hypothesis while keeping this evidence and source history intact.
