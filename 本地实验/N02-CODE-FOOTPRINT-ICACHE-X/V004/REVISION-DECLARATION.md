# N02 V004 Revision Declaration

ROUTE=N02 CODE-FOOTPRINT-ICACHE-X
REVISION=V004
PARENT_REVISION=V003
PARENT_COMMIT=3c7aedd908919be1147c50a68fb05180ff87951b
SOURCE_PARENT_PATH=本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V003/submission.asc
SOURCE_PARENT_SHA256=89c2ecc333baea90359cad61709024b1d61aed213ac0c444d058b8275d9aef20
CANDIDATE_SOURCE=本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V004/submission.asc
CANDIDATE_SOURCE_SHA256=75aef181ceb2822c69fb32107b6cc20ac4dcc88d13a1fd727bb7755785314f2d
CANDIDATE_SOURCE_BYTES=190077
CHANGE=Add __attribute__((noinline)) to AddRmsNormBiasKernel<T>::FromFloat at submission.asc:3397; retain V001 Load noinline and V003 Store noinline.
CALL_PATH=FromFloat is called by existing output conversion paths at submission.asc:440, 483, 590, 607, 727, 845, 978, 1092, 1758, 1797, 2788, 2885, 3033, 3322, and 3336.
SHAPE_PATH=UNCHANGED
ALGORITHM=UNCHANGED
ADDRESS_GENERATION=UNCHANGED
SYNCHRONIZATION=UNCHANGED
PERFORMANCE_HYPOTHESIS=Making the existing FromFloat conversion helper out-of-line may reduce repeated conversion-helper instruction body across output hotspots; only generated kernel artifacts decide whether the compiler materializes a difference.
FORBIDDEN_OVERLAP=R029 mode dispatch; N01 register lifetime; N03 vector ILP; N04 GM/L2 access; N05 scalar address generation.
CONFIG_SCOPE=Reuse the V002 independent Parent/Candidate paired-source mapping and the existing V016 CANN build inputs; no new Runner and no mode dispatch.
REMOTE_TEMP=/tmp/cann-n02-code-footprint-icache-x-v004-sOhSYF
DEVICE=3
RUNNER_SAFETY=Existing paired_runner_v016 hardcodes kDevice=4 and calls aclrtResetDevice(4) on exit; compile is allowed, runtime execution is forbidden while this entry remains unsafe.
RULE_REFRESH_RECEIPT=ROUTE=N02 CODE-FOOTPRINT-ICACHE-X; HEAD=3c7aedd908919be1147c50a68fb05180ff87951b; PARENT=66890e2838db29621d57d038ff5361a6e461f06e; DIRECT_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f; MECHANISM=hotspot helper code footprint and instruction fetch; FORBIDDEN_OVERLAP=R029 mode dispatch, N01 register lifetime, N03 vector ILP, N04 GM/L2 access, N05 scalar address generation; NEXT_ACTION=compile V003/V004 standalone objects and existing paired_runner_v016 with separate paths.
COMPILE=PASS
COMPILE_COMMAND=cmake -S /tmp/cann-n02-code-footprint-icache-x-v004-sOhSYF/single-v003 -B /tmp/cann-n02-code-footprint-icache-x-v004-sOhSYF/single-v003/build; cmake --build /tmp/cann-n02-code-footprint-icache-x-v004-sOhSYF/single-v003/build --target device submission full_link --parallel 1 -- -B; cmake -S /tmp/cann-n02-code-footprint-icache-x-v004-sOhSYF/single-v004 -B /tmp/cann-n02-code-footprint-icache-x-v004-sOhSYF/single-v004/build; cmake --build /tmp/cann-n02-code-footprint-icache-x-v004-sOhSYF/single-v004/build --target device submission full_link --parallel 1 -- -B; cmake -S /tmp/cann-n02-code-footprint-icache-x-v004-sOhSYF/paired -B /tmp/cann-n02-code-footprint-icache-x-v004-sOhSYF/paired/build -DR31B_V011_SOURCE_DIR=/tmp/cann-n02-code-footprint-icache-x-v004-sOhSYF/paired-parent -DR31B_V016_SOURCE_DIR=/tmp/cann-n02-code-footprint-icache-x-v004-sOhSYF/paired-candidate; cmake --build /tmp/cann-n02-code-footprint-icache-x-v004-sOhSYF/paired/build --target paired_runner_v016 --parallel 1 -- -B
COMPILE_INPUT=Paired dependency audit resolved Parent=/tmp/cann-n02-code-footprint-icache-x-v004-sOhSYF/paired-parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc and Candidate=/tmp/cann-n02-code-footprint-icache-x-v004-sOhSYF/paired-candidate/R31B-V016-WIDE-TILE-SEED_kernel.asc.
OBJECT_DIFFERENCE=NONE; V003 and V004 device.cpp.o SHA256=3e3266d0d3abffa5d0a48660f97f9a7c3abf9e8abb485d87617fe97c89c0bb57 and submission adapter.cpp.o SHA256=7df79bf620d54da293e2f689dd2b913d7d46c5035681faf43d66ca6417d09064; each object is 334040 bytes and cmp returns 0.
ALINK_DIFFERENCE=NONE; V003 and V004 device.alink SHA256=998896a58bf828357e6885fb5eab3e41b79fcf70e898baeb3abf126f90be1b9f and submission.alink SHA256=1e13fdfd6df7876db3820b2c299271d82aad5259f0e293ad540dd006a00a3630; each alink is 183144 bytes and cmp returns 0.
RUNNER_DIFFERENCE=YES_BUT_NOT_KERNEL; V003 runner SHA256=4c3fa4eab98a7cee2cfbdcdeda73358c27ab9e08494b27f68002a6f5dd7a798f, size=638888; V004 runner SHA256=33ca668c244e08b3a7d250dffe9c801255ce966d0914769bdaa7da05f00b50a7, size=651616. The paired runner object also differs, but these are wrapper artifacts and are not used as kernel instruction-size evidence.
CORRECTNESS=NOT_RUN
LOCAL=NONE
NEGATIVE_RESULT=V001, V003, and V004 standalone kernel artifacts are byte-identical across the tested Load, Store, and FromFloat noinline variants.
STOP_NOINLINE_VARIANTS=YES; stop adding repeated inline/noinline attributes on this standalone build path.
RESULT=FROMFLOAT_NOINLINE_COMPILED; STANDALONE_CODEGEN_EQUAL; PAIRED_RUNNER_DIFFERENCE_RECORDED_ONLY; NO_RUNTIME_RESULT
PERFORMANCE_CLAIM=NONE
GIT_COMMIT=TO_BE_RECORDED
NEXT_ACTION=Stop repeated noinline variants; do not infer kernel performance from paired runner size.
