# N05 SCALAR-ADDRESS-GENERATION-X V008 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V008
PARENT: 78ff4fa803401fbcbb0e57edb4804a2d79f14816
SOURCE_COMMIT: 8f2f031e3f2884b663793f91a1782c5daabe53da
SOURCE_SHA256: 9a32df08e7054ed71390d4ae3fd85b09433fccd5f032f202d8f8fb36abf12a15
CHAMPION_PARENT_SHA256: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

CHANGE: In ProcessWideLowPrecision pass one, replace the per-unit conditional next-row GM offset expression with an incrementing nextRowOffset cursor. The cursor advances at the existing row carry. GM addresses, row/tile traversal, reduction order, arithmetic, DMA calls, event order, and output semantics are unchanged. No runner source changed.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3. The existing V001 paired CMake and safe runner entry were reused.
CORRECTNESS: PASS, CORRECTNESS_RC=0. Four cases passed: FP16/BF16 at D=12288 and D=32768, rows=16. Parent and Candidate each reported failures=0, with device_reset=NO. The reused runner labels the candidate binary as V001 in its output; the binary was built from the V008 candidate SHA recorded above.
LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Paired d4 run, 4 cases, 21 interleaved pairs per binary, warmup_each=45, device event timing with wall-clock diagnostics, runner RC=0.
LOCAL_CASES: FP16 D=12288 median Parent/Candidate=18.080/15.840 us, paired delta median=-2.680 us; FP16 D=32768=31.100/31.180 us, delta=0.180 us; BF16 D=12288=18.060/20.320 us, delta=-0.200 us; BF16 D=32768=36.300/37.180 us, delta=-0.200 us. Several cases have low central spread, but FP16 D=12288 has CV=1.086 for Parent and a large paired outlier; the complete raw sample set remains unchanged and is insufficient for a score.

DEVICE_ID: d4 / server3 hwnput3 / runtime device=4
FREE_HBM: approximately 6.3 GB (59213/65536 MB used in the pre-run d4 snapshot)
AICORE_LOAD: 59 percent in the pre-run d4 snapshot
OTHER_PROCESS_PRESENT: Existing VLLMEngineCor and VLLMWorker_TP processes were recorded and left unchanged. The N05 runner exited with device_reset=NO.
REMOTE_RUN_DIR: ~/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V008/8f2f031e3f2884b663793f91a1782c5daabe53da

SIMILARITY_NOTE: Main audit reports FULL_SOURCE_SIMILARITY approximately 0.982-0.999 for the shared Champion-derived sources, above the 80 percent target because the Champion主体 is shared. This revision contains executable Scalar address-generation changes only; no renaming, comment-only, formatting-only, or meaningless wrapper changes were used to lower the value. EFFECTIVE_CHANGE_SIMILARITY: not measured in this Route result; Main tracks it separately.

EVIDENCE: compile.log, correctness.raw.log, local.raw.log, pre-compile-device.txt, pre-correctness-device.txt, pre-local-device.txt, post-local-device.txt, upload-sha256.txt, local-artifact-sha256.txt, CMakeCache.txt, and paired_runner_v008-linux-aarch64 are stored in this directory. V001 through V007 remain LOCAL_SCORE=NONE as recorded in their committed results.

NEXT_ACTION: Commit this V008 evidence, then continue only with an executable Scalar address-generation change in ProcessWideLowPrecision. Preserve output, tile traversal, arithmetic, DMA, and event semantics; source commit precedes the next Compile.
