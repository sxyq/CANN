# N02 V008 Revision Declaration

ROUTE=N02 CODE-FOOTPRINT-ICACHE-X
REVISION=V008
PARENT_REVISION=V004_SOURCE_WITH_V007_AUDIT_CONTEXT
PARENT_COMMIT=d0002fbe114d39f4002578676a2190a9968780b1
DIRECT_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
HEAD_AT_RULE_REFRESH=d0002fbe114d39f4002578676a2190a9968780b1
RULE_REFRESH_RECEIPT=ROUTE=N02 CODE-FOOTPRINT-ICACHE-X; HEAD=d0002fbe114d39f4002578676a2190a9968780b1; PARENT=c4a80a9a58b3d3516e65c24ae7b9416e20e9f71d; DIRECT_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f; MECHANISM=function-definition and link order for executable hotspot layout; FORBIDDEN=noinline, section-metadata-only, unsupported basic-block option, invalid wrapper, new Runner, other Route mechanism; NEXT_ACTION=compile V004/V008 and compare text and objdump

SOURCE_PARENT_PATH=本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V004/submission.asc
SOURCE_PARENT_SHA256=75aef181ceb2822c69fb32107b6cc20ac4dcc88d13a1fd727bb7755785314f2d
CANDIDATE_SOURCE_PATH=本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V008/submission.asc
CANDIDATE_SOURCE_SHA256=4a2788e57efe4a0019a68928fec2faaa26642a5e3a56e1ea103a0e19ad5fc1b2
CANDIDATE_SOURCE_BYTES=190077
CHANGE=Reverse the source definition order of the six existing synchronization methods SyncMTE2ToV, SyncVToMTE2, SyncVToMTE3, SyncMTE3ToV, SyncVToS, and SyncSToV. Function bodies, attributes, call sites, arithmetic, DMA operations, event IDs, shape predicates, and synchronization operations are unchanged.
DIFF=V008/diff-from-v004.patch; 63 changed diff lines are only the existing six method blocks moving as a group.
R029_RUNTIME_DISPATCH=NOT_USED
NOINLINE_CHANGE=NONE
SHAPE_PATH=UNCHANGED
ALGORITHM=UNCHANGED
DMA=UNCHANGED
SYNCHRONIZATION_SEMANTICS=UNCHANGED

HOST=hwnput3
REMOTE_TEMP=/tmp/cann-n02-code-footprint-icache-x-v008-order
DEVICE=NONE
ENV_INIT=source /usr/local/Ascend/ascend-toolkit/set_env.sh with nounset disabled
ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest
SOC_TARGET=Ascend910B3
TOOLCHAIN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/ccec_compiler/bin/bisheng
TOOLCHAIN_VERSION=clang 15.0.5
BUILD_INPUT=Existing R31B V016 CMakeLists.txt, device.cpp, adapter.cpp, and link_one.sh; source supplied under the existing R31B-V016-WIDE-TILE-SEED_kernel.asc include name.

=== COMPILE ===
PARENT_CONFIG_RC=0
PARENT_BUILD_RC=0
CANDIDATE_CONFIG_RC=0
CANDIDATE_BUILD_RC=0
COMMAND=cmake -S /tmp/cann-n02-code-footprint-icache-x-v008-order/{parent,candidate} -B .../build; cmake --build .../build --target device submission full_link --parallel 1 -- -B VERBOSE=1

=== OBJECTS AND ALINKS ===
PARENT_DEVICE_OBJECT=3e3266d0d3abffa5d0a48660f97f9a7c3abf9e8abb485d87617fe97c89c0bb57; 334040 bytes
CANDIDATE_DEVICE_OBJECT=3e3266d0d3abffa5d0a48660f97f9a7c3abf9e8abb485d87617fe97c89c0bb57; 334040 bytes
PARENT_SUBMISSION_OBJECT=7df79bf620d54da293e2f689dd2b913d7d46c5035681faf43d66ca6417d09064; 334040 bytes
CANDIDATE_SUBMISSION_OBJECT=7df79bf620d54da293e2f689dd2b913d7d46c5035681faf43d66ca6417d09064; 334040 bytes
PARENT_DEVICE_ALINK=998896a58bf828357e6885fb5eab3e41b79fcf70e898baeb3abf126f90be1b9f; 183144 bytes
CANDIDATE_DEVICE_ALINK=998896a58bf828357e6885fb5eab3e41b79fcf70e898baeb3abf126f90be1b9f; 183144 bytes
PARENT_SUBMISSION_ALINK=1e13fdfd6df7876db3820b2c299271d82aad5259f0e293ad540dd006a00a3630; 183144 bytes
CANDIDATE_SUBMISSION_ALINK=1e13fdfd6df7876db3820b2c299271d82aad5259f0e293ad540dd006a00a3630; 183144 bytes
OBJECT_CMP_RC=0
SUBMISSION_OBJECT_CMP_RC=0
DEVICE_ALINK_CMP_RC=0
SUBMISSION_ALINK_CMP_RC=0

=== TEXT AND OBJDUMP ===
PARENT_HOTSPOT_SECTION=.text._ZN20AddRmsNormBiasKernelIDhE23ProcessWideLowPrecisionEmmjff_mix_aiv; size=4660 bytes; SHA256=662a9e8647eb1960c4e007988b32ece619f2160204f268b287a7e519b65a2d50
CANDIDATE_HOTSPOT_SECTION=.text._ZN20AddRmsNormBiasKernelIDhE23ProcessWideLowPrecisionEmmjff_mix_aiv; size=4660 bytes; SHA256=662a9e8647eb1960c4e007988b32ece619f2160204f268b287a7e519b65a2d50
HOTSPOT_RAW_CMP_RC=0
PARENT_OBJDUMP_SHA256=3f90d601333322b6d14e0a06a1ce2b430b91dc3d39be02a934f9984620467474; 30559 bytes
CANDIDATE_OBJDUMP_SHA256=3f90d601333322b6d14e0a06a1ce2b430b91dc3d39be02a934f9984620467474; 30559 bytes
HOTSPOT_OBJDUMP_CMP_RC=0
PARENT_ALINK_TEXT_SIZE=0x12e1c; RAW_SHA256=e1e893d950450fdde3e22fb7ac239a50bda465aba66200c877649776891cc85a; 77340 bytes
CANDIDATE_ALINK_TEXT_SIZE=0x12e1c; RAW_SHA256=e1e893d950450fdde3e22fb7ac239a50bda465aba66200c877649776891cc85a; 77340 bytes
ALINK_TEXT_CMP_RC=0
FUNCTION_SECTION_ORDER=Compiler-emitted order was identical for Parent/Candidate despite the source definition reorder.

CORRECTNESS=NOT_RUN
LOCAL=NONE
RUNNER=NOT_RUN
SAFETY_REASON=The existing runtime_probe and paired_runner entries call aclrtResetDevice; the requested result was already negative at object/text level, so no device operation was started.
PERFORMANCE_CLAIM=NONE
RESULT=FUNCTION_DEFINITION_ORDER_CODEGEN_EQUAL
NEGATIVE_RESULT=Changing the source definition order of existing synchronization helpers did not change emitted function order, hotspot raw code, objdump, final text, object, or alink bytes.
NEXT_ACTION=STOP_NEW_N02_CANDIDATES; retain this negative result and do not repeat function-order, noinline, or section-metadata variants.
PUSH=NO

EVIDENCE_FILES=V008/diff-from-v004.patch; V008/logs/v008-compile-order.log; V008/logs/v008-artifact-objdump-audit.log; V008/logs/v008-parent-hotspot.objdump; V008/logs/v008-candidate-hotspot.objdump
