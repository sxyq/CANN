# V043 Result

- Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`).
- Change: In the BF16 `ProcessNarrowMidOverlap` epilogue, apply the existing FP32 `invRms` multiply to the FP32 gamma scratch rather than to `valueLocal` after gamma multiplication.
- Compile: PASS for `device` and `submission` targets on `hwnput3` with toolkit `8.5.0.alpha002`.
- Correctness: `PARENT_INVALID_UNRESOLVED`. Both runs dispatched `ProcessNarrowMidOverlap` for BF16 `[128,3072]` with 40 cores and 3-4 rows/core. Parent failed with matched ratio `0.227254232` and max error `5.68677521`; Candidate failed with matched ratio `0.206255595` and max error `6.26680183` (`atol=rtol=1/64`, max-error limit `1.0`). This is not classified as a Candidate regression; the Parent/path failure is unresolved.
- Local: `NOT_RUN_CORRECTNESS_GATE_FAILED`; no score, delta, or samples. `CURRENT_LOCAL_BEST=V026`.
- Correctness runner: the first configure attempt exited `1` because `ASC_DIR`/`CMAKE_PREFIX_PATH` was missing. The retained retry used the discovered ASC package path, built both runners successfully, then returned `1` because both correctness checks failed.
- Device 0: reserved for V043 correctness/result capture. After the result package was captured, the usage snapshot at `2026-10-08T00:22:40Z` showed HBM usage `5%` and AICore usage `0%`; the V043 reservation is explicitly released. No Local run was started.
- Official score absent; Online not run.

Evidence: compile/correctness logs, runner sources, declaration, source hash and diff, JSON results, and the device-release log are retained in this directory.
