# N02 V007 Layout Audit Negative Evidence

ROUTE=N02 CODE-FOOTPRINT-ICACHE-X
REVISION=V007_LAYOUT_AUDIT
PARENT=c4a80a9a58b3d3516e65c24ae7b9416e20e9f71d
DIRECT_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
HEAD_AT_RECEIPT=c4a80a9a58b3d3516e65c24ae7b9416e20e9f71d
RULE_REFRESH_RECEIPT=ROUTE=N02 CODE-FOOTPRINT-ICACHE-X; HEAD=c4a80a9a58b3d3516e65c24ae7b9416e20e9f71d; PARENT=871a770df2f088eba1c25660581f2a8b2b550401; DIRECT_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f; MECHANISM=hotspot executable layout and instruction fetch; FORBIDDEN=repeat noinline, invalid wrapper, dead code, rename-only change, new Runner, R029 runtime dispatch; NEXT_ACTION=stop new Candidate after layout audit

HOST=hwnput3
REMOTE_TEMP=/tmp/cann-n02-code-footprint-icache-x-v007-layout-audit
ENV_INIT=source /usr/local/Ascend/ascend-toolkit/set_env.sh with nounset disabled
ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest
TOOLCHAIN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/ccec_compiler/bin/bisheng
TOOLCHAIN_VERSION=clang 15.0.5
SOC_TARGET=Ascend910B3
SOURCE_USED=V004 submission.asc copied only to the existing V016 include filename
SOURCE_SHA256=75aef181ceb2822c69fb32107b6cc20ac4dcc88d13a1fd727bb7755785314f2d
CANDIDATE_SOURCE=NONE

=== FACTOR 1: FUNCTION SECTIONS ===
COMMAND_VARIANT=cmake -S support -B sections/build -DCMAKE_CXX_FLAGS=-ffunction-sections; cmake --build sections/build --target device submission full_link --parallel 1 -- -B VERBOSE=1
BASE_COMMAND=cmake -S support -B base/build -DCMAKE_CXX_FLAGS=; cmake --build base/build --target device submission full_link --parallel 1 -- -B VERBOSE=1
BASE_CONFIG_RC=0
BASE_BUILD_RC=0
VARIANT_CONFIG_RC=0
VARIANT_BUILD_RC=0
BASE_DEVICE_OBJECT=3e3266d0d3abffa5d0a48660f97f9a7c3abf9e8abb485d87617fe97c89c0bb57; 334040 bytes
VARIANT_DEVICE_OBJECT=bff02734f15b562cd315b08cd8ab3b125f096e9b0068db8e6be917e283795df2; 334888 bytes
BASE_SUBMISSION_OBJECT=7df79bf620d54da293e2f689dd2b913d7d46c5035681faf43d66ca6417d09064; 334040 bytes
VARIANT_SUBMISSION_OBJECT=ee4085a9b97eb6ed7d491ef494edcbea41cf81f1e977e90d9104c46b91f29196; 334888 bytes
BASE_DEVICE_ALINK=998896a58bf828357e6885fb5eab3e41b79fcf70e898baeb3abf126f90be1b9f; 183144 bytes
VARIANT_DEVICE_ALINK=14421a8d9ed251c7a4d78bfa2b85032d45a327ad1c9c6eaa102430a83151252a; 183240 bytes
BASE_SUBMISSION_ALINK=1e13fdfd6df7876db3820b2c299271d82aad5259f0e293ad540dd006a00a3630; 183144 bytes
VARIANT_SUBMISSION_ALINK=1bd88ab2064f11857c5f4fc4a73b63b50b058fcf4c88232c44c7af317cbd2ebb; 183240 bytes
OBJECT_AND_ALINK_BYTE_DIFFERENCE=YES
FUNCTION_CODE_SECTION=The FP16 ProcessWideLowPrecision object section stayed 0x1234 bytes in both builds. The final device.alink .text stayed 0x12e1c bytes in both builds. The extracted raw FP16 section was 4660 bytes in both builds with BASE_SHA256=662a9e8647eb1960c4e007988b32ece619f2160204f268b287a7e519b65a2d50 and VARIANT_SHA256=662a9e8647eb1960c4e007988b32ece619f2160204f268b287a7e519b65a2d50; cmp returned 0.
FACTOR_1_RESULT=Artifact metadata changed; hotspot executable bytes did not change. Rejected as a valid N02 executable-layout Candidate.

=== FACTOR 2: BASIC BLOCK SECTIONS ===
COMMAND_VARIANT=cmake -S support -B basicblocks/build -DCMAKE_CXX_FLAGS=-fbasic-block-sections=all
CONFIG_RC=1
BUILD_RC=2
RAW_ERROR=bisheng: error: unsupported option '-fbasic-block-sections=all' for target 'aarch64-unknown-linux-gnu'
FACTOR_2_RESULT=The existing CANN compiler rejected the option during its CMake compiler probe; no target artifact was generated.

COMPILE=BASE_PASS_AND_FUNCTION_SECTIONS_PASS_METADATA_ONLY; BASIC_BLOCK_LAYOUT_UNSUPPORTED
CORRECTNESS=NOT_RUN
LOCAL=NONE
DEVICE=NONE
RUNNER=NOT_RUN
PERFORMANCE_CLAIM=NONE
RESULT=NO_VALID_SINGLE_FACTOR_EXECUTABLE_LAYOUT_CHANGE_FOUND
STOP=STOP_NEW_N02_CANDIDATES_AFTER_THIS_AUDIT; do not infer benefit from section-table-only differences.

EVIDENCE_FILES=入口审计-bisheng-layout-options.log; V007/logs/v007-layout-flag-audit.log; V007/logs/v007-layout-section-audit.log; V007/logs/v007-layout-code-section-audit.log; V007/logs/v007-layout-disassembly-audit.log; V007/logs/v007-layout-raw-section-audit.log
