#!/usr/bin/env bash
# Server3 reproducibility harness for Phase4 best sources.
# Does NOT modify candidate sources in-repo. Compiles exact online/local snapshots.
set -uo pipefail

CANN_ROOT="${CANN_ROOT:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
REMOTE_HOME="${REMOTE_HOME:-/home/data4t2/lelinfeng/phase4-review-repro}"
LOCAL_ROOT="${LOCAL_ROOT:-/Users/sunyiyang/Desktop/Project/cann}"

set +u
source "${CANN_ROOT}/bin/setenv.bash" >/dev/null 2>&1 || source "${CANN_ROOT}/set_env.sh" >/dev/null 2>&1 || true
set -u
export ASCEND_HOME_PATH="${CANN_ROOT}"
export CMAKE_PREFIX_PATH="${CANN_ROOT}/aarch64-linux/tikcpp/ascendc_kernel_cmake${CMAKE_PREFIX_PATH:+:${CMAKE_PREFIX_PATH}}"
export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"
export C_INCLUDE_PATH="/usr/include:/usr/include/aarch64-linux-gnu${C_INCLUDE_PATH:+:${C_INCLUDE_PATH}}"

BISHENG="${CANN_ROOT}/aarch64-linux/ccec_compiler/bin/bisheng"
LLD="${CANN_ROOT}/tools/ccec_compiler/bin/ld.lld"
[ -x "${LLD}" ] || LLD="${CANN_ROOT}/aarch64-linux/ccec_compiler/bin/ld.lld"
HCC="${CANN_ROOT}/toolkit/toolchain/hcc"

echo "ENV CANN=${CANN_ROOT}"
echo "ENV BISHENG=$([ -x "$BISHENG" ] && echo OK || echo MISSING)"
echo "ENV LLD=$([ -x "$LLD" ] && echo OK || echo MISSING)"
echo "ENV SOC=Ascend910B3 NPU_ARCH=dav-2201"

mkdir -p "${REMOTE_HOME}"
mkdir -p "${LOCAL_ROOT}/phase4/review/logs"

# rows: group|route|revision|local_src_path
# historical uses phase4/online/.../submission.asc
# local uses phase4/local/.../submission.asc
COMPILE_ONE() {
  local group="$1" route="$2" rev="$3" rel="$4"
  local name="${route}-${rev}"
  local src_local="${LOCAL_ROOT}/${rel}"
  local src_name="submission.asc"
  local work="${REMOTE_HOME}/${name}"
  local log="${LOCAL_ROOT}/phase4/review/logs/${name}.log"

  echo "======== ${group} ${name} ========"
  {
    echo "ROUTE=${route}"
    echo "REVISION=${rev}"
    echo "GROUP=${group}"
    echo "SOURCE_LOCAL=${rel}"

    if [[ ! -f "${src_local}" ]]; then
      echo "DEVICE_COMPILE=SKIP_SOURCE_MISSING"
      echo "SUBMISSION_COMPILE=SKIP_SOURCE_MISSING"
      echo "FULL_LINK=SKIP_SOURCE_MISSING"
      echo "REPRO_STATUS=TOOLING_BLOCKED"
      return
    fi

    local sha
    sha=$(sha256sum "${src_local}" | awk '{print $1}')
    echo "SOURCE_SHA256=${sha}"

    ssh -o BatchMode=yes cann-server3 "mkdir -p '${work}/src' '${work}/build'" || {
      echo "DEVICE_COMPILE=SKIP_SSH"
      echo "SUBMISSION_COMPILE=SKIP_SSH"
      echo "FULL_LINK=SKIP_SSH"
      echo "REPRO_STATUS=TOOLING_BLOCKED"
      return
    }
    scp -q -o BatchMode=yes "${src_local}" "cann-server3:${work}/src/${src_name}" || {
      echo "DEVICE_COMPILE=SKIP_SCP"
      echo "REPRO_STATUS=TOOLING_BLOCKED"
      return
    }

    # Detect TensorInfo need
    local needs_adapter=0
    if ! grep -q 'struct TensorInfo' "${src_local}"; then
      needs_adapter=1
    fi
    echo "NEEDS_TENSORINFO_ADAPTER=${needs_adapter}"

    # Generate remote adapter + device stub + CMake via python on server3
    ssh -o BatchMode=yes cann-server3 "python3 - <<'PY'
import os, pathlib
work=pathlib.Path(r'''${work}''')
src=work/'src'/'${src_name}'
text=src.read_text()
needs=${needs_adapter}
adapter_types='''
#include <cstdint>
#ifndef TENSOR_GROUP_INFO_DEFINED
#define TENSOR_GROUP_INFO_DEFINED
struct TensorInfo { const int64_t* shape; int64_t numDims; int32_t dtype; };
struct TensorGroupInfo { const TensorInfo* tensors; int64_t numTensors; };
#endif
using aclrtStream = void*;
'''
if needs:
    body = adapter_types + '#include \"${src_name}\"\nint main(){return 0;}\n'
else:
    # self-contained may still need main for host link of submission object
    if 'int main' not in text and 'int main(' not in text:
        body = adapter_types + '#include \"${src_name}\"\nint main(){return 0;}\n'
    else:
        body = text
    if 'struct TensorInfo' in text:
        body = '#include <cstdint>\n#include \"${src_name}\"\n#ifndef TENSOR_GROUP_INFO_DEFINED\n#define TENSOR_GROUP_INFO_DEFINED\nstruct TensorInfo { const int64_t* shape; int64_t numDims; int32_t dtype; };\nstruct TensorGroupInfo { const TensorInfo* tensors; int64_t numTensors; };\n#endif\nusing aclrtStream = void*;\nint main(){return 0;}\n'
        # wait - if source already defines TensorInfo, only add main
        body = '#include \"${src_name}\"\nint main(){return 0;}\n' if 'struct TensorInfo' in text else body
(work/'src'/'adapter.cpp').write_text(body if needs else ('#include \"${src_name}\"\nint main(){return 0;}\n' if 'int main' not in text else '#include \"${src_name}\"\n'))
(work/'src'/'device_stub.asc').write_text('#include \"${src_name}\"\n' if needs else text if False else '#include \"${src_name}\"\n')
# always device from direct source include for aicore-only
(work/'src'/'device_include.asc').write_text('#include \"${src_name}\"\n')
(work/'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.16)
set(SOC_VERSION Ascend910B3 CACHE STRING \"soc\")
set(CCE_AICORE_ARCH dav-2201 CACHE STRING \"arch\")
set(ASCEND_CANN_PACKAGE_PATH \"${CANN_ROOT}\" CACHE PATH \"cann\")
set(ASCENDC_INCLUDE_DIR \"${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include\")
set(ASCENDC_INTERNAL_INCLUDE_DIR \"${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include\")
set(ASCENDC_IMPL_INCLUDE_DIR \"${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include/basic_api/impl\")
set(ASCENDC_PUBLIC_BASIC_INCLUDE_DIR \"${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/basic_api\")
set(ASCENDC_PUBLIC_INCLUDE_DIR \"${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include\")
set(ASCENDC_ROOT_INCLUDE_DIR \"${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux\")
set(ASCENDC_ASC_ROOT_DIR \"${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc\")
set(ASCENDC_PUBLIC_INTERFACE_INCLUDE_DIR \"${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/interface\")
set(HCC_TOOLCHAIN \"${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc\")
set(BISHENG \"${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ccec_compiler/bin/bisheng\")
set(CMAKE_CXX_COMPILER \"${BISHENG}\" CACHE FILEPATH \"bisheng\" FORCE)
project(repro LANGUAGES CXX)
function(add_flags tgt)
  target_include_directories(\${tgt} PRIVATE
    \"${ASCENDC_INCLUDE_DIR}\" \"${ASCENDC_INTERNAL_INCLUDE_DIR}\" \"${ASCENDC_IMPL_INCLUDE_DIR}\"
    \"${ASCENDC_ROOT_INCLUDE_DIR}\" \"${ASCENDC_ASC_ROOT_DIR}\" \"${ASCENDC_PUBLIC_INCLUDE_DIR}\"
    \"${ASCENDC_PUBLIC_BASIC_INCLUDE_DIR}\" \"${ASCENDC_PUBLIC_INTERFACE_INCLUDE_DIR}\"
    \"${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include\"
    \"${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0\"
    \"${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu\"
    \"${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward\"
    \${CMAKE_CURRENT_SOURCE_DIR}/src)
  target_compile_options(\${tgt} PRIVATE \"--cce-aicore-lang\" \"--cce-aicore-arch=dav-c220-vec\"
    \"--cce-aicore-only\" \"--cce-auto-sync\" \"--npu-arch=dav-2201\" \"--npu-soc=Ascend910B3\" \"-std=c++17\")
endfunction()
add_library(device OBJECT src/device_include.asc)
add_flags(device)
add_library(submission OBJECT src/adapter.cpp)
add_flags(submission)
add_executable(full_link src/adapter.cpp)
add_flags(full_link)
target_link_options(full_link PRIVATE -L/usr/lib/gcc/aarch64-linux-gnu/11 -L/usr/lib/aarch64-linux-gnu)
target_link_libraries(full_link PRIVATE stdc++ m dl)
set_target_properties(full_link PROPERTIES LINKER_LANGUAGE CXX POSITION_INDEPENDENT_CODE OFF)
set_target_properties(device submission PROPERTIES POSITION_INDEPENDENT_CODE OFF)
'''.replace('${CANN_ROOT}', '''${CANN_ROOT}'''))
print('WROTE', work)
PY"

    # Fix: pass CANN_ROOT into remote python properly — rewrite CMake with actual path
    ssh -o BatchMode=yes cann-server3 "sed -i 's|ASCEND_CANN_PACKAGE_PATH.*CACHE PATH|ASCEND_CANN_PACKAGE_PATH \"${CANN_ROOT}\" CACHE PATH|' '${work}/CMakeLists.txt'; ls -la '${work}/src'; cat '${work}/src/adapter.cpp' | head -20"

    echo "=== device compile ==="
    ssh -o BatchMode=yes cann-server3 "set -e; source ${CANN_ROOT}/bin/setenv.bash >/dev/null 2>&1 || true; cd '${work}'; rm -rf build; mkdir build; cmake -S . -B build -DCMAKE_BUILD_TYPE=Release >build/cfg.log 2>&1; cmake --build build --target device -j2 >build/device.log 2>&1"; DEV=$?
    echo "DEVICE_RC=${DEV}"

    echo "=== submission compile ==="
    ssh -o BatchMode=yes cann-server3 "set -e; source ${CANN_ROOT}/bin/setenv.bash >/dev/null 2>&1 || true; cd '${work}'; cmake --build build --target submission -j2 >build/submission.log 2>&1"; SUB=$?
    echo "SUBMISSION_RC=${SUB}"

    echo "=== full link ==="
    ssh -o BatchMode=yes cann-server3 "set -e; source ${CANN_ROOT}/bin/setenv.bash >/dev/null 2>&1 || true; cd '${work}'; cmake --build build --target full_link -j2 >build/link.log 2>&1"; LNK=$?
    echo "FULL_LINK_RC=${LNK}"

    # fetch logs
    scp -q -o BatchMode=yes "cann-server3:${work}/build/cfg.log" "${log}.cfg" 2>/dev/null || true
    scp -q -o BatchMode=yes "cann-server3:${work}/build/device.log" "${log}.device" 2>/dev/null || true
    scp -q -o BatchMode=yes "cann-server3:${work}/build/submission.log" "${log}.submission" 2>/dev/null || true
    scp -q -o BatchMode=yes "cann-server3:${work}/build/link.log" "${log}.link" 2>/dev/null || true

    local status=REPRODUCIBLE
    if [[ ${DEV} -ne 0 ]]; then status=COMPILE_FAIL; fi
    if [[ ${DEV} -eq 0 && ${SUB} -ne 0 ]]; then status=COMPILE_FAIL; fi
    if [[ ${DEV} -eq 0 && ${SUB} -eq 0 && ${LNK} -ne 0 ]]; then status=LINK_FAIL; fi
    if [[ ${DEV} -ne 0 || ${SUB} -ne 0 || ${LNK} -ne 0 ]]; then
      echo "DIAGNOSTIC=$(tail -n 15 ${log}.device ${log}.submission ${log}.link 2>/dev/null | tr '\\n' ' ' | cut -c1-400)"
    else
      echo "DIAGNOSTIC="
    fi
    echo "REPRO_STATUS=${status}"

    # cleanup remote objects optional
    ssh -o BatchMode=yes cann-server3 "find '${work}/build' -type f \\( -name '*.o' -o -name '*.d' \\) -delete 2>/dev/null || true" || true
  } 2>&1 | tee "${log}"
}

# Historical online bests
COMPILE_ONE HISTORICAL R31B V011 phase4/online/R31B/V011/submission.asc
COMPILE_ONE HISTORICAL R31A V016 phase4/online/R31A/V016/submission.asc
COMPILE_ONE HISTORICAL MIX-A V003 phase4/online/MIX-A/V003/submission.asc
COMPILE_ONE HISTORICAL A001 V017 phase4/online/A001/V017/submission.asc
COMPILE_ONE HISTORICAL G001 V002 phase4/online/G001/V002/submission.asc
COMPILE_ONE HISTORICAL H001 V008 phase4/online/H001/V008/submission.asc
COMPILE_ONE HISTORICAL I001 V004 phase4/online/I001/V004/submission.asc
# Current local exploration
COMPILE_ONE LOCAL WIDE-X-FRESH4 CURRENT phase4/local/WIDE-X-FRESH4/CURRENT/submission.asc
COMPILE_ONE LOCAL MODE-X-R015C CURRENT phase4/local/MODE-X-R015C/CURRENT/submission.asc
COMPILE_ONE LOCAL EPI-X-FRESH CURRENT phase4/local/EPI-X-FRESH/CURRENT/submission.asc

echo "ALL_DONE"
