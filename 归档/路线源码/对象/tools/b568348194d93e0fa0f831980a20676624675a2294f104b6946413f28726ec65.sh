#!/usr/bin/env bash
# Generic single-file AscendC kernel compile verifier for server3.
set -u
TOOLKIT="${TOOLKIT:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
BASE="/home/data4t2/lelinfeng/server_full_audit"
export ASCEND_HOME_PATH="${TOOLKIT}"
export ASCEND_CANN_PACKAGE_PATH="${TOOLKIT}"
export CMAKE_PREFIX_PATH="${TOOLKIT}/aarch64-linux/tikcpp/ascendc_kernel_cmake${CMAKE_PREFIX_PATH:+:${CMAKE_PREFIX_PATH}}"
export PATH="${TOOLKIT}/bin:${TOOLKIT}/compiler/ccec_compiler/bin:${TOOLKIT}/tools/ccec_compiler/bin:${PATH}"
export LIBRARY_PATH=/usr/lib/gcc/aarch64-linux-gnu/11:/usr/lib/aarch64-linux-gnu:/lib/aarch64-linux-gnu:${LIBRARY_PATH:-}
export LD_LIBRARY_PATH="${TOOLKIT}/aarch64-linux/lib64:${TOOLKIT}/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common:${LD_LIBRARY_PATH:-}"
export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"

SRC_DIR="${BASE}/src"
OUT="${BASE}/out"
LOG="${BASE}/logs"
WORKROOT="${BASE}/work"
mkdir -p "$OUT" "$LOG" "$WORKROOT"

compile_one() {
  local safe="$1"
  local src="${SRC_DIR}/${safe}.asc"
  local work="${WORKROOT}/${safe}"
  local result="${OUT}/${safe}.tsv"
  local log="${LOG}/${safe}.build.log"
  mkdir -p "${work}"
  rm -rf "${work}/build"
  cat > "${work}/CMakeLists.txt" <<EOF
cmake_minimum_required(VERSION 3.16)
find_package(ASC REQUIRED)
project(k_${safe} LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 17)
ascendc_library(k SHARED "${src}")
target_compile_options(k PRIVATE \$<\$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_compile_options(k PRIVATE \$<\$<COMPILE_LANGUAGE:ASC>:--npu-soc=Ascend910B3>)
target_compile_options(k PRIVATE \$<\$<COMPILE_LANGUAGE:ASC>:-include> \$<\$<COMPILE_LANGUAGE:ASC>:/home/data4t2/lelinfeng/server_full_audit/judge_abi.h>)
target_include_directories(k PRIVATE "${BASE}/src" "${BASE}/inc" "${TOOLKIT}/include" "${TOOLKIT}/aarch64-linux/include" "$(dirname ${src})")
target_link_directories(k PRIVATE "${TOOLKIT}/lib64" "${TOOLKIT}/aarch64-linux/lib64")
target_link_libraries(k PRIVATE ascendcl)
target_link_directories(k PRIVATE
  /usr/lib/gcc/aarch64-linux-gnu/11
  /usr/lib/aarch64-linux-gnu
  /lib/aarch64-linux-gnu
  "${TOOLKIT}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/lib64")
target_link_options(k PRIVATE -Wl,-rpath-link,/usr/lib/gcc/aarch64-linux-gnu/11)
EOF
  local rc=0
  local t0=$(date +%s)
  cmake -S "${work}" -B "${work}/build" -DCMAKE_BUILD_TYPE=Release \
    -DASCEND_HOME_PATH="${TOOLKIT}" \
    -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH}" >"${log}" 2>&1 || rc=1
  if [[ $rc -eq 0 ]]; then
    cmake --build "${work}/build" --parallel 2 >>"${log}" 2>&1 || rc=1
  fi
  local t1=$(date +%s)
  local so=""
  so=$(find "${work}/build" -name 'libk.so' -o -name 'libk.a' 2>/dev/null | head -1)
  local so_sha="-"
  if [[ -n "$so" && -f "$so" ]]; then
    so_sha=$(sha256sum "$so" | awk '{print $1}')
    so="${so}"
  else
    so="-"
    rc=1
  fi
  printf '%s\t%s\t%s\t%s\t%s\n' "$safe" "$rc" "$((t1-t0))" "${so}" "${so_sha}" > "$result"
  echo "DONE $safe rc=$rc dt=$((t1-t0))s" >> "${LOG}/progress.log"
}

JOBS="${JOBS:-12}"
mapfile -t ALL < <(ls "${SRC_DIR}"/*.asc | xargs -n1 basename | sed 's/\.asc$//' \
  | grep -E 'V[0-9]{3}|CURRENT|BUILD-FIX|r[0-9]' | grep -E 'V[0-9]{3}$|CURRENT$|BUILD-FIX-001$|R015C-r[0-9]$|^r[0-9]+$' | sort)
echo "TOTAL_JOBS=${#ALL[@]} JOBS=$JOBS host=$(hostname) start=$(date -Is)"

running=0
for safe in "${ALL[@]}"; do
  compile_one "$safe" &
  running=$((running+1))
  if [[ $running -ge $JOBS ]]; then
    wait -n 2>/dev/null || wait
    running=$((running-1))
  fi
done
wait
echo "ALL_DONE $(date -Is) results=$(ls "$OUT" | wc -l)"
