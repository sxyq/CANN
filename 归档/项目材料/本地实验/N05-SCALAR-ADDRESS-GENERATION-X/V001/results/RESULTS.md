# N05 SCALAR-ADDRESS-GENERATION-X V001 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V001
PARENT: 1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
SOURCE_SHA256: 63cf041baaba1e1dfdc95c103173856f19e3b0447bd207b51ddcc06dc27bdce1
SOURCE_COMMIT: 47c59a77156799f9f58f83e1bcfd1bb8013309f3
SUPPORT_COMMIT: 54cbd11a2c5f02bfcb06d3ed736c1d6dbf560f5f

CHANGE: In ProcessWideLowPrecision pass one, replace per-tile u division and repeated address arithmetic with row, tile, and column cursors. Tile order, arithmetic, DMA, and event order are unchanged.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002. The first environment attempt failed with missing ASCEND_HOME_PATH (RC=2); the environment-fixed attempt failed in the plugin compile with missing <vector> (RC=2). The final build used the recorded C++ include paths and passed.
CORRECTNESS: PASS, CORRECTNESS_RC=0. Four cases passed: FP16/BF16 at D=12288 and D=32768, rows=16. Parent and Candidate each reported failures=0.
LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Paired d4 run, 4 cases, 21 interleaved pairs per binary, warmup_each=45, device event timing with wall-clock diagnostics, runner RC=0.
LOCAL_OBSERVATION: Raw samples contain large load-driven outliers. BF16 D=32768 device medians were Parent=30.62 us and Candidate=32.10 us; paired delta median=-0.50 us and MAD=19.58 us; CV exceeded 1. The sample set is retained without converting it to a score.

DEVICE_ID: d4 / server3 hwnput3 / runtime device=4
FREE_HBM: approximately 6.3 GB during the recorded runs
AICORE_LOAD: approximately 59-61 percent
OTHER_PROCESS_PRESENT: VLLMEngineCor PID 2999855 and other pre-existing VLLM processes; none were changed. N05 runner exited with device_reset=NO.
REMOTE_RUN_ROOT: ~/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V001/54cbd11a2c5f02bfcb06d3ed736c1d6dbf560f5f
EVIDENCE: compile-first.log, compile-envfixed-failed.log, compile-passed.log, correctness-loader-failed.raw.log, correctness-passed.raw.log, local.raw.log, device snapshots, CMakeCache.txt, and runner build artifacts in this directory.

NEXT_ACTION: Start V002 from this Candidate and change only the ProcessWideLowPrecision pass-two Scalar row/column address generation. Preserve output, tile traversal, arithmetic, DMA, and event semantics; then commit source before CANN Compile, Correctness, and Local.
