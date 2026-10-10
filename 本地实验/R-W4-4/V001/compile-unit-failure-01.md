# Compile attempt 01 — failed

- COMPILE_START_TIMESTAMP: 2026-10-06T17:50:41Z
- COMPILER_DIAGNOSTIC: `bisheng: error: cannot compile ASC file and CCE file together.`
- INTERPRETATION: Compile FAIL. The shell pipeline returned 0 because `tee` masked the compiler exit status; that value is not a compile pass. The raw `compile.log` is preserved unchanged.
- FIX_SCOPE: compile adapter input extension/invocation only; candidate source unchanged.
- CORRECTNESS: NOT_RUN
- LOCAL/PERFORMANCE: NOT_RUN (W4 gate suspended)
- ONLINE: FORBIDDEN
