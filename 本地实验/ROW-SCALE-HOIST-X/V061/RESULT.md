# ROW-SCALE-HOIST-X V061 Result

- Parent: exact V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`).
- Candidate source: `f0a00ce8ac8165704401b41a91d90a1016aa3d5ffa53f97e508613bad5d1008d`.
- Single change: FP32 wide aligned-panel row-scale placement using row-stride gamma/bias chunks, then per-row `invRms`, before the existing store.
- Compile: PASS for `device` and `submission`.
- Correctness runner support fix: explicit `ASCEND_HOME_PATH`, `ASCEND_CANN_PACKAGE_PATH`, `ASC_DIR`, HCC toolchain, `CPLUS_INCLUDE_PATH`, and `C_INCLUDE_PATH`; fresh direct Parent/Candidate runner build at `20261008T184836Z` PASS.
- Target dispatch: `ProcessWideFp32FullCacheRows`, FP32 `[128,12288]`, device 3.
- Fresh direct-run Parent correctness: FAIL; matched ratio `0.222640991`, max absolute error `4.45902586`.
- Fresh direct-run Candidate correctness diagnostic: FAIL; matched ratio `0.217860540`, max absolute error `4.76071262`.
- Blocker: fresh direct runner build and rerun confirm exact V026 Parent does not pass the target FP32 wide correctness gate. The prior FP16 PASS logs are non-target and excluded.
- Local: NOT RUN; no numeric Local Score is claimed because correctness failed.
- Current Local Best remains V026. V061 is not promoted and must not become a V062 Parent.
- Raw logs, environment, build, device context, and source diff are indexed in `raw-evidence-index.tsv`.
