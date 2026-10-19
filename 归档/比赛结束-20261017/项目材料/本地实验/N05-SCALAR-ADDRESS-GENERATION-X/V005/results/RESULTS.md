# N05 SCALAR-ADDRESS-GENERATION-X V005 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V005
PARENT: 9f0bd92f1dff81987aacfff6dd1447b1688e8b7c
SOURCE_COMMIT: 1c66a1bd6f2a95d92a89e8fe5f2bed331675b04b
SOURCE_SHA256: b518d392ff1967a20262ae14b303397d4eceec7a26216a995718d9166871da92
CHAMPION_PARENT_SHA256: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

CHANGE: In ProcessWideLowPrecision pass one, add an incrementing local value-row offset that advances with the existing row cursor. The two retained y/value accesses use valueRowOffset + col instead of batchRow * rowWidth + col. GM addresses, row/tile traversal, arithmetic, DMA calls, event order, and output semantics are unchanged. No runner source changed.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3. The existing V001 paired CMake and safe runner entry were reused.
CORRECTNESS: PASS, CORRECTNESS_RC=0. Four cases passed: FP16/BF16 at D=12288 and D=32768, rows=16. Parent and Candidate each reported failures=0, with device_reset=NO. The reused runner labels the candidate binary as V001 in its output; the binary was built from the V005 candidate SHA recorded above.
LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Paired d4 run, 4 cases, 21 interleaved pairs per binary, warmup_each=45, device event timing with wall-clock diagnostics, runner RC=0.
LOCAL_CASES: FP16 D=12288 median Parent/Candidate=18.120/23.140 us, paired delta median=2.480 us; FP16 D=32768=31.160/30.060 us, delta=0.620 us; BF16 D=12288=20.800/20.340 us, delta=0.560 us; BF16 D=32768=38.520/36.580 us, delta=-2.360 us. FP16 D=12288 and both BF16 cases had CV above 1 for both binaries; paired differences contained large outliers. The complete raw sample set remains unchanged.

DEVICE_ID: d4 / server3 hwnput3 / runtime device=4
FREE_HBM: approximately 6.3 GB (59213/65536 MB used in the pre-run d4 snapshot)
AICORE_LOAD: 61 percent in the pre-run d4 snapshot
OTHER_PROCESS_PRESENT: Existing VLLMEngineCor and VLLMWorker_TP processes were recorded and left unchanged. The N05 runner exited with device_reset=NO.
REMOTE_RUN_DIR: ~/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V005/1c66a1bd6f2a95d92a89e8fe5f2bed331675b04b

EVIDENCE: compile.log, correctness.raw.log, local.raw.log, pre-compile-device.txt, pre-correctness-device.txt, pre-local-device.txt, post-local-device.txt, upload-sha256.txt, local-artifact-sha256.txt, CMakeCache.txt, and paired_runner_v005-linux-aarch64 are stored in this directory. V001 through V004 remain LOCAL_SCORE=NONE as recorded in their committed results.

NEXT_ACTION: Commit this V005 evidence, then continue with one independent Scalar address-generation change in ProcessWideLowPrecision. Preserve output, tile traversal, arithmetic, DMA, and event semantics; source commit precedes the next Compile.
