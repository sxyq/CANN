# Compile attempt 04 — failed

- COMPILE_START_TIMESTAMP: 2026-10-06T18:02:35Z (end timestamp from compile log)
- RESULT: device target compiled; host adapter target failed.
- DIAGNOSTIC: `TensorGroupInfo`, `TensorInfo`, and `aclrtStream` were undeclared in `src/adapter.cpp`.
- FIX_SCOPE: compile-wrapper declarations only; no candidate-source or dispatch-threshold change.
- CORRECTNESS: NOT_RUN
- LOCAL/PERFORMANCE: NOT_RUN (W4 gate suspended)
- ONLINE: FORBIDDEN
