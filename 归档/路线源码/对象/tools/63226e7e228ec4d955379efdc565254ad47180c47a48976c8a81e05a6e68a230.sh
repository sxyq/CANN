#!/bin/bash
# AddRmsNormBias 真机编译入口。
# 当前目录是源码清单；真正的 CANN 编译必须在 msopgen 生成工程中进行。
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GENERATED_PROJECT_DIR="${ASCENDC_GENERATED_PROJECT_DIR:-${1:-}}"

if ! command -v msopgen >/dev/null 2>&1; then
    echo "[ERROR] 未找到 msopgen；需要在真实 CANN 环境执行。" >&2
    exit 2
fi

if [ -z "${GENERATED_PROJECT_DIR}" ] || [ ! -f "${GENERATED_PROJECT_DIR}/CMakeLists.txt" ]; then
    echo "[ERROR] 未提供 msopgen 生成工程。" >&2
    echo "用法：ASCENDC_GENERATED_PROJECT_DIR=/path/to/generated ${SCRIPT_DIR}/build.sh" >&2
    exit 2
fi

if [ -z "${ASCEND_CANN_PACKAGE_PATH:-}" ]; then
    echo "[ERROR] 未设置 ASCEND_CANN_PACKAGE_PATH。" >&2
    exit 2
fi

BUILD_DIR="${GENERATED_PROJECT_DIR}/build"
BUILD_JOBS="${BUILD_JOBS:-1}"
cmake -S "${GENERATED_PROJECT_DIR}" -B "${BUILD_DIR}" \
      -DASCEND_CANN_PACKAGE_PATH="${ASCEND_CANN_PACKAGE_PATH}" \
      -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}" --parallel "${BUILD_JOBS}"
echo "[OK] 已完成 msopgen 工程编译：${BUILD_DIR}"
