# ROW-SCALE-HOIST-X V061

- Direct parent: V026, exact parent source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Single change: in the FP32 wide aligned panel path, apply gamma and bias by column chunk with the existing row-stride vector form, then apply each row's `invRms` before the existing store path.
- Preserved: dispatch, reduction, dtype policy, core ownership, UB allocation, cross-row pipeline, and all non-target paths.
- Target case: FP32 `[128,12288]`, device 3, Ascend910B3, 40 vector cores, `ProcessWideFp32FullCacheRows`.
- Candidate source SHA-256: `f0a00ce8ac8165704401b41a91d90a1016aa3d5ffa53f97e508613bad5d1008d`.
- Compile: PASS for `device` and `submission`; the V061 candidate input is preserved under `compile/`.
- Correctness runner support fix: explicit `ASCEND_HOME_PATH`, `ASCEND_CANN_PACKAGE_PATH`, `ASC_DIR`, HCC toolchain, and C/C++ include paths; the fresh runner build passed.
- Correctness: BLOCKED. The exact V026 Parent failed the target FP32 wide correctness check; the V061 Candidate also failed in diagnostic execution. See `correctness-result.json` and the timestamped raw logs.
- Local: NOT RUN because the Parent correctness gate failed. No Local Score is claimed.
- Current Local Best remains V026. V061 is not a valid performance Parent for V062.
- No shared-record edit, Online action, or other Route change.
