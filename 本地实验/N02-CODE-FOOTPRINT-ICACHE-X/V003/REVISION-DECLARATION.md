# N02 V003 Revision Declaration

ROUTE=N02 CODE-FOOTPRINT-ICACHE-X
REVISION=V003
PARENT_REVISION=V002
PARENT_COMMIT=c1b7f57c1e49a5051270a73faf7f256b94bf1c2b
SOURCE_PARENT_REVISION=V001
SOURCE_PARENT_PATH=本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V001/submission.asc
SOURCE_PARENT_SHA256=2238d0ff3604f0de752d9d9ecb5546f0ec94aa2205b9018853cdd19d7ca04fd1
CANDIDATE_SOURCE=本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V003/submission.asc
CANDIDATE_SOURCE_SHA256=89c2ecc333baea90359cad61709024b1d61aed213ac0c444d058b8275d9aef20
CANDIDATE_SOURCE_BYTES=190058
CHANGE=Add __attribute__((noinline)) to AddRmsNormBiasKernel<T>::Store at submission.asc:3377; retain V001 Load noinline at submission.asc:3367.
CALL_PATH=Store is called by existing output paths at submission.asc:468, 480, 489, 587, 598, 610, 731, 854, 982, 1100, 1218, 1452, 1554, 1660, 1773, 1800, 1922, 2224, 2374, 2516, 2796, 2890, 3041, and 3341.
SHAPE_PATH=UNCHANGED
ALGORITHM=UNCHANGED
ADDRESS_GENERATION=UNCHANGED
SYNCHRONIZATION=UNCHANGED
PERFORMANCE_HYPOTHESIS=Making the frequently called Store helper out-of-line may reduce repeated store-helper instruction body at output call sites; call overhead and compiler behavior are measured from generated artifacts.
FORBIDDEN_OVERLAP=R029 mode dispatch; N01 register lifetime; N03 vector ILP; N04 GM/L2 access; N05 scalar address generation.
CONFIG_SCOPE=Reuse the V002 independent Parent/Candidate paired-runner path mapping; no new Runner and no mode dispatch.
REMOTE_TEMP=/tmp/cann-n02-code-footprint-icache-x-v003-i68uFx
DEVICE=3
RUNNER_SAFETY=Existing paired_runner_v016 hardcodes kDevice=4 and calls aclrtResetDevice(4) on exit; compile is allowed, runtime execution is conditional on that safety issue and will not proceed while it remains.
RULE_REFRESH_RECEIPT=ROUTE=N02 CODE-FOOTPRINT-ICACHE-X; HEAD=c1b7f57c1e49a5051270a73faf7f256b94bf1c2b; PARENT=031fcc5351b25eafa4f6ce04bd426f3f487fdb7b; DIRECT_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f; MECHANISM=hotspot helper code footprint and instruction fetch; FORBIDDEN_OVERLAP=R029 mode dispatch, N01 register lifetime, N03 vector ILP, N04 GM/L2 access, N05 scalar address generation; NEXT_ACTION=compile V001/V003 standalone objects and existing paired_runner_v016 with separate paths.
COMPILE=PASS_AFTER_INPUT_FIX
COMPILE_ATTEMPTS=The first standalone configure failed because the existing V016 CMake input referenced runtime_probe.cpp that was not staged; the existing runtime_probe.cpp was then uploaded and V001/V003 device submission full_link targets both configured and built with return code 0. The paired_runner_v016 target configured and built with return code 0 on the first attempt.
COMPILE_COMMAND=cmake -S /tmp/cann-n02-code-footprint-icache-x-v003-i68uFx/single-v001 -B /tmp/cann-n02-code-footprint-icache-x-v003-i68uFx/single-v001/build; cmake --build /tmp/cann-n02-code-footprint-icache-x-v003-i68uFx/single-v001/build --target device submission full_link --parallel 1 -- -B; cmake -S /tmp/cann-n02-code-footprint-icache-x-v003-i68uFx/single-v003 -B /tmp/cann-n02-code-footprint-icache-x-v003-i68uFx/single-v003/build; cmake --build /tmp/cann-n02-code-footprint-icache-x-v003-i68uFx/single-v003/build --target device submission full_link --parallel 1 -- -B; cmake -S /tmp/cann-n02-code-footprint-icache-x-v003-i68uFx/paired -B /tmp/cann-n02-code-footprint-icache-x-v003-i68uFx/paired/build -DR31B_V011_SOURCE_DIR=/tmp/cann-n02-code-footprint-icache-x-v003-i68uFx/paired-parent -DR31B_V016_SOURCE_DIR=/tmp/cann-n02-code-footprint-icache-x-v003-i68uFx/paired-candidate; cmake --build /tmp/cann-n02-code-footprint-icache-x-v003-i68uFx/paired/build --target paired_runner_v016 --parallel 1 -- -B
OBJECT_DIFFERENCE=NONE; V001 and V003 device.cpp.o SHA256=3e3266d0d3abffa5d0a48660f97f9a7c3abf9e8abb485d87617fe97c89c0bb57, submission adapter.cpp.o SHA256=7df79bf620d54da293e2f689dd2b913d7d46c5035681faf43d66ca6417d09064; each object size is 334040 bytes and cmp returns 0.
ALINK_DIFFERENCE=NONE; V001 and V003 device.alink SHA256=998896a58bf828357e6885fb5eab3e41b79fcf70e898baeb3abf126f90be1b9f, submission.alink SHA256=1e13fdfd6df7876db3820b2c299271d82aad5259f0e293ad540dd006a00a3630; each alink size is 183144 bytes and cmp returns 0.
RUNNER_DIFFERENCE=YES; V002 runner SHA256=9a0966dd23c716927b73d9eafd21d993a477c2c2ed855ea2a1032681d87cf43b, size=625984; V003 runner SHA256=4c3fa4eab98a7cee2cfbdcdeda73358c27ab9e08494b27f68002a6f5dd7a798f, size=638888; paired runner object SHA256 changed from 5309600fd1b63dd522eca9a2e31f7a81e7bc4abe64ca293b4c761557937d7dd6, size=315776, to d797c95e91dbe69e670e68ae3e764ea2c712a12df957cf14f322972d381fa4e0, size=328752.
CORRECTNESS=NOT_RUN
CORRECTNESS_REASON=The existing paired runner hardcodes device 4 and calls aclrtResetDevice(4); runtime execution was not issued.
LOCAL=NONE
LOCAL_REASON=Compile-only request; no device-event sample was collected.
RESULT=STORE_NOINLINE_COMPILED; STANDALONE_CODEGEN_EQUAL; PAIRED_RUNNER_CODEGEN_DIFFERENT; NO_RUNTIME_RESULT
GIT_COMMIT=TO_BE_RECORDED
EVIDENCE=logs/v003-preflight-compile-device3.log; logs/v003-envfix-standalone-device3.log; logs/v003-artifact-audit.log; diff-from-v001.patch
NEXT_ACTION=No runtime inference from V003; retain generated artifact differences for the next N02 decision.
