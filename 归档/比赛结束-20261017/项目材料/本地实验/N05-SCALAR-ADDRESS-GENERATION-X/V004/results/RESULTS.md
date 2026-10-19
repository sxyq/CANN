# N05 SCALAR-ADDRESS-GENERATION-X V004 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V004
PARENT: bb3a6ddf5ee2f297ae670ed3d06e3476b317b743
SOURCE_COMMIT: 9f0bd92f1dff81987aacfff6dd1447b1688e8b7c
SOURCE_SHA256: f0a7d8454a66c81cf1ea70f57e5e48d0320d58ad0bed8e20b0945b578322d2e5
CHAMPION_PARENT_SHA256: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

CHANGE: In ProcessWideLowPrecision pass two, initialize incrementing local value-row offsets and use valueRowOffsets[batchRow] + col for retained y/value access. The GM output row cursor from V002 and parameter tile col cursor from V003 remain unchanged. Tile order, arithmetic, DMA calls, event order, and output semantics are unchanged. No runner source changed.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3. The existing V001 paired CMake and safe runner entry were reused.
CORRECTNESS: PASS, CORRECTNESS_RC=0. Four cases passed: FP16/BF16 at D=12288 and D=32768, rows=16. Parent and Candidate each reported failures=0, with device_reset=NO. The reused runner labels the candidate binary as V001 in its output; the binary was built from the V004 candidate SHA recorded above.
LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Paired d4 run, 4 cases, 21 interleaved pairs per binary, warmup_each=45, device event timing with wall-clock diagnostics, runner RC=0.
LOCAL_CASES: FP16 D=12288 median Parent/Candidate=19.200/19.040 us, paired delta median=1.520 us; FP16 D=32768=32.020/31.680 us, delta=-0.340 us; BF16 D=12288=20.040/18.720 us, delta=-3.580 us; BF16 D=32768=37.020/39.980 us, delta=-0.100 us. Parent CV exceeded 1 in all four cases and paired differences contained large outliers. The complete raw sample set remains unchanged.

DEVICE_ID: d4 / server3 hwnput3 / runtime device=4
FREE_HBM: approximately 6.3 GB (59213/65536 MB used in the pre-run d4 snapshot)
AICORE_LOAD: 58 percent in the pre-run d4 snapshot
OTHER_PROCESS_PRESENT: Existing VLLMEngineCor and VLLMWorker_TP processes were recorded and left unchanged. The N05 runner exited with device_reset=NO.
REMOTE_RUN_DIR: ~/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V004/9f0bd92f1dff81987aacfff6dd1447b1688e8b7c

EVIDENCE: compile.log, correctness.raw.log, local.raw.log, pre-compile-device.txt, pre-correctness-device.txt, pre-local-device.txt, post-local-device.txt, upload-sha256.txt, local-artifact-sha256.txt, CMakeCache.txt, and paired_runner_v004-linux-aarch64 are stored in this directory. V001, V002, and V003 remain LOCAL_SCORE=NONE as recorded in their committed results.

NEXT_ACTION: Commit this V004 evidence, then continue with one independent Scalar address-generation change in ProcessWideLowPrecision. Preserve output, tile traversal, arithmetic, DMA, and event semantics; source commit precedes the next Compile.
