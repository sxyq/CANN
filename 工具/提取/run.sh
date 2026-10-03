#!/usr/bin/env bash
# CANN 群聊提取 — 一键入口
#
# 用法:
#   ./run.sh              增量提取 + 筛选（推荐日常使用）
#   ./run.sh --full       全量重新提取
#   ./run.sh --status     查看增量状态
#   ./run.sh --reset      清除增量状态
#   ./run.sh --setup      首次安装依赖
#
# 前提：手机已连 USB、开启 USB 调试、已授予 root（KernelSU/Magisk）

set -euo pipefail
cd "$(dirname "$0")"

PY="${CANN_PYTHON:-}"
if [[ -z "$PY" ]]; then
  # 优先用带 sqlcipher3 的解释器
  for cand in python3 /Users/sunyiyang/.workbuddy/binaries/python/envs/default/bin/python; do
    if command -v "$cand" >/dev/null 2>&1 && \
       "$cand" -c "import sqlcipher3" 2>/dev/null; then
      PY="$cand"; break
    fi
  done
fi
if [[ -z "$PY" ]]; then
  PY="$(command -v python3 || true)"
fi
if [[ -z "$PY" ]]; then
  echo "✗ 未找到 python3"
  exit 1
fi

if [[ "${1:-}" == "--setup" ]]; then
  echo "▸ 安装依赖…"
  "$PY" -m pip install sqlcipher3 || {
    echo "  标准轮子不可用，尝试 macOS arm64 预编译轮子…"
    "$PY" -m pip install --no-cache-dir \
      "https://files.pythonhosted.org/packages/56/0d/2cee40de57d47245de09382c64e649c8cc8e86fa549ecba7591633fabf20/sqlcipher3-0.6.2-cp313-cp313-macosx_11_0_arm64.whl"
  }
  "$PY" -c "import sqlcipher3; print('✓ sqlcipher3 就绪')"
  exit 0
fi

exec "$PY" cann_extract.py "$@"
