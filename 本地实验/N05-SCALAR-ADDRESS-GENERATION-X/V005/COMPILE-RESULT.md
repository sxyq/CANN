# N05 V005 Compile

ROUTE=N05-SCALAR-ADDRESS-GENERATION-X
REVISION=V005
PARENT=V004
PARENT_COMMIT=0020dafb77638d4cd4283037d240f4bce8d29eb8
COMPILE=NOT_RUN
CONFIGURE_RC=NOT_RUN
BUILD_RC=NOT_RUN
FINAL_RC=NOT_RUN
SSH_RC=NOT_APPLICABLE
REMOTE_HOST=cann-server3
REMOTE_DIR=/tmp/n05-scalar-address-generation-v005.jrdB8C
SOC_VERSION=Ascend910B3
ENV_SOURCE=/usr/local/Ascend/ascend-toolkit/set_env.sh
CMAKE_ENTRY=本地实验/R31B/V016/support/CMakeLists.txt
CMAKE_INPUTS=existing V016 CMakeLists.txt,device.cpp,adapter.cpp,link_one.sh,runtime_probe.cpp; no CMake invocation after duplicate decision
TARGETS=device,submission,full_link
PROPOSED_SOURCE_LOCAL_BYTES=190730
PROPOSED_SOURCE_SHA256=8f5aae77cc549527109ebdcf104c799f81d29052492f434d043f6f85c2e46ed7
SOURCE_STATUS=RETAINED_FOR_AUDIT_ONLY
DUPLICATE_KNOWN_PARENT=YES
KNOWN_PARENT_EVIDENCE=本地实验/N05-SCALAR-ADDRESS-GENERATION-X/V001/diff.patch:16-17
RAW_LOG=compile-attempt-01.log
RESOURCE_LOG=compile-attempt-01.log
FIRST_FAILURE=DUPLICATE_KNOWN_PARENT
RUNTIME_RUN=NONE
REMOTE_TEMP_ALLOCATED=YES
REMOTE_BUILD_PROCESS=NONE_FOUND_AFTER_STOP
REMOTE_TEMP_CLEANUP=PASS; REMOTE_TEMP_CLEANUP_RC=0

COMPILE_INVOCATION=NOT_RUN
COMPILE_STOP=USER_REQUEST_AFTER_DUPLICATE_KNOWN_PARENT

The proposed flat-unit formulas match the known pre-V004 address path recorded in V001. The remote temporary directory was allocated by the interrupted preparation, no V005 build process remained, and that exact temporary directory was removed. No CMake, device build, or runtime command was completed.
