ROUTE: STAGING-LIVENESS-X
REVISION: V001
DIRECT_PARENT: R31B-V011
PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA: c11cd5b27a89ca1d30aee180a09ac7796aa1aa83e8ff2ead512f2ea82fe87e49

## Compile record

- Final result: COMPILE PASS at `2026-10-06T18:04:28Z`.
- Scope: CMake `device` and `submission` object targets only. `full_link` was not run.
- CANN root: `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`; detected host SoC family: Ascend910B3.
- Configure log: `server_runs/STAGING-LIVENESS-X/V001/build/configure.log`.
- Device compile log: `server_runs/STAGING-LIVENESS-X/V001/build/device-compile.log`.
- Submission compile log: `server_runs/STAGING-LIVENESS-X/V001/build/submission-compile.log`.
- Device compile passed on the first actual build attempt. Submission object compile initially failed because the copied adapter lacked judge-template declarations for `TensorInfo`, `TensorGroupInfo`, and `aclrtStream`. The only Build Fix added those wrapper declarations to `src/adapter.cpp`; candidate kernel source and its SHA were unchanged. Recompile then passed both targets.
- A prior invocation ended before configure because of a shell variable-separator error; it was not counted as a compile result.
- Correctness: NOT RUN. Local performance: SUSPENDED_FOR_W4. Online: FORBIDDEN.
- No correctness, performance, or online result is claimed.
