# N05 SCALAR-ADDRESS-GENERATION-X V006 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V006
PARENT: 1c66a1bd6f2a95d92a89e8fe5f2bed331675b04b
SOURCE_COMMIT: 1e2ba118e90b2d98f2df64d61a914f3a965cc1fb
SOURCE_SHA256: 9b0faccd8183c5e76ac8ccb5053b40234e5d8aa5853676c6f21a2ce307948e40
CHAMPION_PARENT_SHA256: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

CHANGE: In ProcessWideLowPrecision pass one, replace reduction output indexing batchRow * reduceStride + tile with an incrementing Scalar reduceRowOffset + tile. The offset advances at the existing row carry. GM addresses, row/tile traversal, arithmetic, DMA calls, event order, and output semantics are unchanged. No runner source changed.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3. The existing V001 paired CMake and safe runner entry were reused.
CORRECTNESS: PASS, CORRECTNESS_RC=0. Four cases passed: FP16/BF16 at D=12288 and D=32768, rows=16. Parent and Candidate each reported failures=0, with device_reset=NO. The reused runner labels the candidate binary as V001 in its output; the binary was built from the V006 candidate SHA recorded above.
LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Paired d4 run, 4 cases, 21 interleaved pairs per binary, warmup_each=45, device event timing with wall-clock diagnostics, runner RC=0.
LOCAL_CASES: FP16 D=12288 median Parent/Candidate=22.560/15.260 us, paired delta median=-2.080 us; FP16 D=32768=18.340/19.560 us, delta=-0.160 us; BF16 D=12288=10.840/10.780 us, delta=-0.120 us; BF16 D=32768=33.260/104.540 us, delta=-0.640 us. All cases contain large outliers; device CV was above 1 for both binaries in three cases and for the Candidate in the fourth. The complete raw sample set remains unchanged.

DEVICE_ID: d4 / server3 hwnput3 / runtime device=4
FREE_HBM: approximately 6.3 GB (59213/65536 MB used in the pre-run d4 snapshot)
AICORE_LOAD: 52 percent in the pre-run d4 snapshot
OTHER_PROCESS_PRESENT: Existing VLLMEngineCor and VLLMWorker_TP processes were recorded and left unchanged. The N05 runner exited with device_reset=NO.
REMOTE_RUN_DIR: ~/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V006/1e2ba118e90b2d98f2df64d61a914f3a965cc1fb

SIMILARITY_NOTE: Main audit reports FULL_SOURCE_SIMILARITY approximately 0.982-0.999 for the ten shared Champion-derived sources, above the 80 percent target because the Champion主体 is shared. This revision contains executable Scalar address-generation changes only; no renaming, comment-only, formatting-only, or meaningless wrapper changes were used to lower the value. EFFECTIVE_CHANGE_SIMILARITY: not measured in this Route result; Main tracks it separately.

EVIDENCE: compile.log, correctness.raw.log, local.raw.log, pre-compile-device.txt, pre-correctness-device.txt, pre-local-device.txt, post-local-device.txt, upload-sha256.txt, local-artifact-sha256.txt, CMakeCache.txt, and paired_runner_v006-linux-aarch64 are stored in this directory. V001 through V005 remain LOCAL_SCORE=NONE as recorded in their committed results.

NEXT_ACTION: Commit this V006 evidence, then continue only with an executable Scalar address-generation change in ProcessWideLowPrecision. Preserve output, tile traversal, arithmetic, DMA, and event semantics; source commit precedes the next Compile.
