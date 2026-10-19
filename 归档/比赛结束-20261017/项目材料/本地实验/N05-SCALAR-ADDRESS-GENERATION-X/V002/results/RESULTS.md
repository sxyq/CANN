# N05 SCALAR-ADDRESS-GENERATION-X V002 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V002
PARENT: 47c59a77156799f9f58f83e1bcfd1bb8013309f3
SOURCE_COMMIT: 59e60d9d424fed045ab6c4e337d5931117c2d337
SOURCE_SHA256: 3eb7aba17c72c0c0f226d2d8fc42435d53e4bc9b04fd29ccb4674b82b6f13742
CHAMPION_PARENT_SHA256: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

CHANGE: In ProcessWideLowPrecision pass two, initialize one incrementing GM row-offset array from batchBegin * rowWidth and advance it by rowWidth. Stores use storeRowOffsets[batchRow] + col instead of recomputing (batchBegin + batchRow) * rowWidth + col. Tile loop order, arithmetic, DMA calls, event order, and output semantics are unchanged. No runner source changed.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3. The existing V001 paired CMake and safe runner entry were reused. The CMake configure warning about libstdc++ search paths did not prevent the build.
CORRECTNESS: PASS, CORRECTNESS_RC=0. Four cases passed: FP16/BF16 at D=12288 and D=32768, rows=16. Parent and Candidate each reported failures=0, with device_reset=NO. The reused runner labels the candidate binary as V001 in its output; the binary was built from the V002 candidate SHA recorded above.
LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Paired d4 run, 4 cases, 21 interleaved pairs per binary, warmup_each=45, device event timing with wall-clock diagnostics, runner RC=0.
LOCAL_CASES: FP16 D=12288 median Parent/Candidate=9.920/10.100 us, paired delta median=0.160 us; FP16 D=32768=22.200/77.360 us, delta=0.660 us; BF16 D=12288=20.980/20.960 us, delta=0.320 us; BF16 D=32768=28.680/27.960 us, delta=-1.240 us. All four cases had device CV > 1 for at least one binary, with large outliers; the raw samples remain unchanged.

DEVICE_ID: d4 / server3 hwnput3 / runtime device=4
FREE_HBM: approximately 6.3 GB (59213/65536 MB used in the recorded d4 snapshots)
AICORE_LOAD: 58 percent in the pre-run d4 snapshot
OTHER_PROCESS_PRESENT: Existing VLLMEngineCor and VLLMWorker_TP processes were recorded and left unchanged. The N05 runner exited with device_reset=NO.
REMOTE_RUN_DIR: ~/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V002/59e60d9d424fed045ab6c4e337d5931117c2d337

EVIDENCE: compile.log, correctness.raw.log, local.raw.log, pre-compile-device.txt, pre-correctness-device.txt, pre-local-device.txt, post-local-device.txt, upload-sha256.txt, CMakeCache.txt, and paired_runner_v002-linux-aarch64 are stored in this directory. V001 remains LOCAL_SCORE=NONE as recorded in its committed results.

NEXT_ACTION: Commit this V002 evidence, then start V003 with one independent Scalar address-generation change in the same ProcessWideLowPrecision axis. Preserve output, tile traversal, arithmetic, DMA, and event semantics; source commit precedes the next Compile.
