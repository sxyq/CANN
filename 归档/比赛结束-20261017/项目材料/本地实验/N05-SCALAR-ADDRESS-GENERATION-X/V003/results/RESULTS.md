# N05 SCALAR-ADDRESS-GENERATION-X V003 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V003
PARENT: 59e60d9d424fed045ab6c4e337d5931117c2d337
SOURCE_COMMIT: bb3a6ddf5ee2f297ae670ed3d06e3476b317b743
SOURCE_SHA256: 65f397d0d5b551e5993fe131228d37e4bbf33d63395b2ee9ea73045e3cd98f7a
CHAMPION_PARENT_SHA256: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

CHANGE: In ProcessWideLowPrecision pass two, replace the parameter-tile column expression tile * tileWidth with an incrementing Scalar col cursor. The cursor advances once after each existing tile iteration. Tile order, arithmetic, DMA calls, event order, and output semantics are unchanged. No runner source changed.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3. The existing V001 paired CMake and safe runner entry were reused.
CORRECTNESS: PASS, CORRECTNESS_RC=0. Four cases passed: FP16/BF16 at D=12288 and D=32768, rows=16. Parent and Candidate each reported failures=0, with device_reset=NO. The reused runner labels the candidate binary as V001 in its output; the binary was built from the V003 candidate SHA recorded above.
LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Paired d4 run, 4 cases, 21 interleaved pairs per binary, warmup_each=45, device event timing with wall-clock diagnostics, runner RC=0.
LOCAL_CASES: FP16 D=12288 median Parent/Candidate=10.240/10.040 us, paired delta median=-0.380 us; FP16 D=32768=58.620/56.620 us, delta=24.480 us; BF16 D=12288=11.580/11.340 us, delta=0.600 us; BF16 D=32768=26.540/26.740 us, delta=-0.020 us. FP16 cases and BF16 D=12288 had CV above 1 for one or both binaries, with large outliers. The complete raw sample set remains unchanged.

DEVICE_ID: d4 / server3 hwnput3 / runtime device=4
FREE_HBM: approximately 6.3 GB (59212/65536 MB used in the pre-run d4 snapshot)
AICORE_LOAD: 60 percent in the pre-run d4 snapshot
OTHER_PROCESS_PRESENT: Existing VLLMEngineCor and VLLMWorker_TP processes were recorded and left unchanged. The N05 runner exited with device_reset=NO.
REMOTE_RUN_DIR: ~/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V003/bb3a6ddf5ee2f297ae670ed3d06e3476b317b743

EVIDENCE: compile.log, correctness.raw.log, local.raw.log, pre-compile-device.txt, pre-correctness-device.txt, pre-local-device.txt, post-local-device.txt, upload-sha256.txt, local-artifact-sha256.txt, CMakeCache.txt, and paired_runner_v003-linux-aarch64 are stored in this directory. V001 and V002 remain LOCAL_SCORE=NONE as recorded in their committed results.

NEXT_ACTION: Commit this V003 evidence, then continue with one independent Scalar address-generation change in ProcessWideLowPrecision. Preserve output, tile traversal, arithmetic, DMA, and event semantics; source commit precedes the next Compile.
