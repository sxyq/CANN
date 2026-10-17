# N05 SCALAR-ADDRESS-GENERATION-X V007 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V007
PARENT: 1e2ba118e90b2d98f2df64d61a914f3a965cc1fb
SOURCE_COMMIT: 78ff4fa803401fbcbb0e57edb4804a2d79f14816
SOURCE_SHA256: 6c26ac328b6650baa110ee94cd1fe5ff41c7ed2493fce0e344c088f848cda12b
CHAMPION_PARENT_SHA256: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

CHANGE: In ProcessWideLowPrecision pass one, replace the batch-row reduction start address batchRow * reduceStride with an incrementing reduceBatchOffset. The offset advances once per existing batch-row reduction. GM addresses, row/tile traversal, reduction order, arithmetic, DMA calls, event order, and output semantics are unchanged. No runner source changed.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3. The existing V001 paired CMake and safe runner entry were reused.
CORRECTNESS: PASS, CORRECTNESS_RC=0. Four cases passed: FP16/BF16 at D=12288 and D=32768, rows=16. Parent and Candidate each reported failures=0, with device_reset=NO. The reused runner labels the candidate binary as V001 in its output; the binary was built from the V007 candidate SHA recorded above.
LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Paired d4 run, 4 cases, 21 interleaved pairs per binary, warmup_each=45, device event timing with wall-clock diagnostics, runner RC=0.
LOCAL_CASES: FP16 D=12288 median Parent/Candidate=17.080/15.940 us, paired delta median=-1.200 us; FP16 D=32768=28.660/27.860 us, delta=-1.820 us; BF16 D=12288=18.420/19.640 us, delta=1.420 us; BF16 D=32768=35.200/44.840 us, delta=3.240 us. FP16 cases and BF16 D=32768 contain CV above 1 and large outliers; the complete raw sample set remains unchanged.

DEVICE_ID: d4 / server3 hwnput3 / runtime device=4
FREE_HBM: approximately 6.3 GB (59212/65536 MB used in the pre-run d4 snapshot)
AICORE_LOAD: 60 percent in the pre-run d4 snapshot
OTHER_PROCESS_PRESENT: Existing VLLMEngineCor and VLLMWorker_TP processes were recorded and left unchanged. The N05 runner exited with device_reset=NO.
REMOTE_RUN_DIR: ~/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V007/78ff4fa803401fbcbb0e57edb4804a2d79f14816

SIMILARITY_NOTE: Main audit reports FULL_SOURCE_SIMILARITY approximately 0.982-0.999 for the ten shared Champion-derived sources, above the 80 percent target because the Champion主体 is shared. This revision contains executable Scalar address-generation changes only; no renaming, comment-only, formatting-only, or meaningless wrapper changes were used to lower the value. EFFECTIVE_CHANGE_SIMILARITY: not measured in this Route result; Main tracks it separately.

EVIDENCE: compile.log, correctness.raw.log, local.raw.log, pre-compile-device.txt, pre-correctness-device.txt, pre-local-device.txt, post-local-device.txt, upload-sha256.txt, local-artifact-sha256.txt, CMakeCache.txt, and paired_runner_v007-linux-aarch64 are stored in this directory. V001 through V006 remain LOCAL_SCORE=NONE as recorded in their committed results.

NEXT_ACTION: Commit this V007 evidence, then continue only with an executable Scalar address-generation change in ProcessWideLowPrecision. Preserve output, tile traversal, arithmetic, DMA, and event semantics; source commit precedes the next Compile.
