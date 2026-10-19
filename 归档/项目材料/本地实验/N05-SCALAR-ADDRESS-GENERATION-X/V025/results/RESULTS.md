# N05 SCALAR-ADDRESS-GENERATION-X V025 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V025
PARENT: V024 / source commit f2ad29fdcbe1d030c0aeeb66eddab50f198162fd
SOURCE_COMMIT: 57601e93369371893f33e69148f0e13a30c953a9

CHANGE: In ProcessWideLowPrecision pass two, remove the per-row Store offset cursor. Each output GM row address is derived from batchBaseOffset + batchRow * rowWidth + col. Store order and count, arithmetic, tile order, DMA/event order, Core ownership, and Vector emission remain unchanged.

SOURCE_IDENTITY: Parent V024 local and remote source SHA256 are both 4216181d19b2315a33903384f56ea58a09a361c6fd2cb2c3f375c81c8c65b328. Candidate V025 local and remote source SHA256 are both 8f121f14dca82adfeb837e2abff342912b6f1530b53be31616cfd2342209d3de. CMakeCache binds independent Parent and Candidate directories. The built Runner SHA256 is cdae8802fd4719479d30fda96d0070178faeb8da8aba19889d0b45529980778c and matches the saved local binary.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3 / Ascend910B3. The existing CANN 8.5 setenv.bash initialized the toolchain; ASCEND_HOME_PATH, GCC 11 CPLUS_INCLUDE_PATH, and CPATH were set. The paired CMake target built successfully.

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate on d2; eight binary/case checks passed; CORRECTNESS_RC=0; device_reset=NO. Runtime used the established N05 CANN LD_LIBRARY_PATH and LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6.

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Existing paired Runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. The complete log contains 84 SAMPLE rows and RUNNER_COMPLETE mode=PAIRED cases=4 rows=16. No samples were removed or selected.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta min..max (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 12.780 | 11.680 | +0.420 | -25.040..48.060 | 21 |
| FP16 D=32768 | 20.580 | 19.840 | -1.560 | -22.460..9.280 | 21 |
| BF16 D=12288 | 13.900 | 18.760 | +2.780 | -88.600..9.480 | 21 |
| BF16 D=32768 | 28.600 | 34.100 | +0.440 | -26.960..32.520 | 21 |

LOCAL_VALIDITY: NONE. All paired-delta ranges span zero, and paired medians have mixed signs. BF16 D=12288 has a wide range and the FP16 D=12288 Candidate has an elevated maximum; d2 was under shared load. The data do not establish a stable gain or a reliable single-case signal.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2. Before Correctness, HBM was 62855/65536 MB and AICore was 5%; before Local, HBM was 62855/65536 MB and AICore was 26%. Python PID 816979 using 59487 MB on d2 remained present and was not modified. Pre/post snapshots are retained. device_reset=NO.

REMOTE_RUN_DIR: /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V025/8f121f14-v024-parent

EVIDENCE: configure.log, CMakeCache.txt, compile.log, correctness.raw.log, local.raw.log, paired_runner_v025-linux-aarch64, build-identity.txt, and pre/post device snapshots are stored here.

NEXT_ACTION: Continue with a distinct Scalar address-generation factor. Preserve arithmetic, Store order, tile order, DMA/event order, Core ownership, and Vector scheduling.
