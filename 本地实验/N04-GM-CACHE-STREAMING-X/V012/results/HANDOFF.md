# N04 V012 Handoff

V012 is a single-factor L2 hint trial based directly on N04 V011. It changes only the wide low-precision `residualGm_` hint from `CACHE_MODE_LAST` to `CACHE_MODE_NORMAL`; `xGm_` stays `CACHE_MODE_PERSISTENT` and output stays `CACHE_MODE_DISABLE`. Candidate source commit is `5216bad0d`, source SHA-256 is `0863241837520aec1d42cd1a3bcea7bec3fbad82b05aa4fad43aa2283f401600`.

Using the existing server3 CMake entry and `paired_runner_v016`, final Compile passed, Correctness passed 4/4 on d7, and Champion V011/V012 Local completed 4x21 with Runner return code 0. Every shape had 21/21 negative paired deltas; `LOCAL_SCORE=INDICATIVE_GAIN` for this one run. Full inputs, environment failures and recovery, raw samples, statistics, and d7 snapshots are recorded in `RESULTS.md` and adjacent files. All failed build and launch logs remain separate and intact.

Next single-factor hypothesis: derive V013 from V012 and change only `xGm_` from `CACHE_MODE_PERSISTENT` to `CACHE_MODE_DISABLE`; retain residual `CACHE_MODE_NORMAL`, output `CACHE_MODE_DISABLE`, and all compute/dataflow behavior. This tests the x stream under V012's residual policy without repeating the V006 `xGm_ CACHE_MODE_LAST` choice. Use the existing paired Runner with R31B V011 Champion and measure only on d7.
