# N05 SCALAR-ADDRESS-GENERATION-X V026 Results

ROUTE: N05-SCALAR-ADDRESS-GENERATION-X
REVISION: V026
PARENT: V025 / source commit 57601e93369371893f33e69148f0e13a30c953a9
SOURCE_COMMIT: 1fbb6af47078e92057c0686ec96892e6db903472

CHANGE: In ProcessWideLowPrecision pass two, derive the Scalar tile column as tile * tileWidth instead of advancing col after each tile. The tile loop and tile sequence remain unchanged. Arithmetic, parameter and value addressing semantics, DMA/event order, Core ownership, and Vector emission remain unchanged.

SOURCE_IDENTITY: Parent V025 local and remote source SHA256 are both 8f121f14dca82adfeb837e2abff342912b6f1530b53be31616cfd2342209d3de. Candidate V026 local and remote source SHA256 are both b10e3160e682d2e24a264a3ec842d14f213babdd75f192418587ab2192c1d094. CMakeCache binds separate Parent and Candidate directories. The built Runner SHA256 is 091092a6dd3c8f6c8fcb5bf2852768fc194d00ba89d4dfd64391da4863637142 and matches the saved local binary.

COMPILE: PASS, BUILD_RC=0. CANN 8.5.0.alpha002 on hwnput3 / Ascend910B3. The existing CANN 8.5 setenv.bash initialized the toolchain; ASCEND_HOME_PATH, GCC 11 CPLUS_INCLUDE_PATH, and CPATH were set. The paired CMake target built successfully.

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate on d2; eight binary/case checks passed; CORRECTNESS_RC=0; device_reset=NO. Runtime used the established N05 CANN LD_LIBRARY_PATH and LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6.

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
LOCAL_METHOD: Existing paired Runner on d2; four cases, 21 interleaved pairs per case, warmup_each=45, device-event timing with wall-clock diagnostics. The complete log has 84 SAMPLE rows and RUNNER_COMPLETE mode=PAIRED cases=4 rows=16. No samples were removed or selected.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta min..max (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 10.280 | 12.040 | +1.940 | -9.520..18.600 | 21 |
| FP16 D=32768 | 18.320 | 18.240 | -0.100 | -12.260..3.060 | 21 |
| BF16 D=12288 | 11.040 | 10.420 | -0.280 | -10.180..5.180 | 21 |
| BF16 D=32768 | 26.300 | 33.860 | +0.120 | -22.820..28.680 | 21 |

LOCAL_VALIDITY: NONE. All paired-delta ranges span zero. The four medians have mixed signs and are small relative to the observed ranges; BF16 D=32768 has broad variation. The data do not establish a stable gain or a reliable single-case signal.

DEVICE_ID: d2 / server3 hwnput3 / runtime device=2. Before Correctness, HBM was 62855/65536 MB and AICore was 7%; before Local, HBM was 62855/65536 MB and AICore was 7%. Python PID 816979 using 59487 MB on d2 remained present and was not modified. Pre/post snapshots are retained. device_reset=NO.

REMOTE_RUN_DIR: /home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V026/b10e3160-v025-parent

EVIDENCE: configure.log, CMakeCache.txt, compile.log, correctness.raw.log, local.raw.log, paired_runner_v026-linux-aarch64, build-identity.txt, and pre/post device snapshots are stored here.

NEXT_ACTION: Continue with a distinct Scalar address-generation factor. Preserve arithmetic, tile order, DMA/event order, Core ownership, and Vector scheduling.
