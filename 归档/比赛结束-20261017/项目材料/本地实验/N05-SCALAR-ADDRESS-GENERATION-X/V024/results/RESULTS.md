# N05 SCALAR-ADDRESS-GENERATION-X V024 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V024
PARENT: V023 / source commit d5e070374e9f21890326916c5b07cf687a5959ce
SOURCE_COMMIT: f2ad29fdcbe1d030c0aeeb66eddab50f198162fd

CHANGE: In ProcessWideLowPrecision, derive the row's reduction-scratch summary address as batchRow * reduceStride in the scalar reduction pass. The reduceBatchOffset cursor and increment are removed. Reduction row order, arithmetic, tile order, DMA/event order, Core ownership, and Vector emission remain unchanged.

SOURCE_IDENTITY: Parent V023 local and remote source SHA256 are both d548d7e4719f983f6c66c844912c1ada03bad15bfe38ce5101f189be6213b62b. Candidate V024 local and remote source SHA256 are both 4216181d19b2315a33903384f56ea58a09a361c6fd2cb2c3f375c81c8c65b328. CMakeCache binds independent Parent and Candidate directories. The built runner SHA256 is a8f9e5f2f76498cbb18d081c1dfc644fd1bbb66f36b3e74ecaae8b8708599612 and matches the saved local runner.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3 / Ascend910B3. The existing CANN 8.5 setenv.bash initialized the toolchain; ASCEND_HOME_PATH and both CPLUS_INCLUDE_PATH and CPATH were set to the GCC 11 host include directories. Toolkit initialization emitted permission warnings for unrelated OPP paths, then the existing paired target built successfully. No source change or rebuild retry was needed.

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate on d2; CORRECTNESS_RC=0; device_reset=NO. Runtime used the existing N05 CANN LD_LIBRARY_PATH and LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6.

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Existing paired runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. The complete log has 84 SAMPLE rows and RUNNER_COMPLETE mode=PAIRED cases=4 rows=16; no samples were removed or selected.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta min..max (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 16.540 | 13.820 | -0.340 | -44.600..15.020 | 21 |
| FP16 D=32768 | 26.480 | 25.480 | +0.800 | -1649.460..11.900 | 21 |
| BF16 D=12288 | 12.200 | 20.320 | +0.360 | -12.880..13.080 | 21 |
| BF16 D=32768 | 25.920 | 28.600 | +1.020 | -2.660..12.720 | 21 |

LOCAL_VALIDITY: NONE. All four paired-delta ranges span zero and medians have mixed signs. FP16 D=32768 contains an extreme -1649.460 us paired delta; d2 was under shared load, with AICore at 18% before Local and HBM at 62855/65536 MB. The measurements do not support a stable gain or an isolated case signal.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2. Before Correctness, HBM was 62855/65536 MB and AICore was 23%; before Local, HBM was 62855/65536 MB and AICore was 18%. PID 816979 (python, 59487 MB on d2) remained present in all snapshots and was not modified. Pre/post snapshots are retained. device_reset=NO.

REMOTE_RUN_DIR: /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V024/4216181d-v023-parent

EVIDENCE: configure.log, CMakeCache.txt, compile.log, correctness.raw.log, local.raw.log, paired_runner_v024-linux-aarch64, build-identity.txt, and pre/post device snapshots are stored here.

NEXT_ACTION: Continue the Scalar address-generation study with a different single factor. Preserve arithmetic, reduction order, tile order, DMA/event order, Core ownership, and Vector scheduling.
