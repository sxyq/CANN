#!/usr/bin/env bash
set -euo pipefail

: "${SYNC_RUNNER:?set SYNC_RUNNER to the built timing executable}"
: "${SYNC_RESULT_DIR:?set SYNC_RESULT_DIR to a fresh result directory}"
: "${CANN_ROOT:=/home/data4t2/lelinfeng/cann}"

if [[ -e "$SYNC_RESULT_DIR" ]]; then
  printf 'RESULT_DIR_EXISTS %s\n' "$SYNC_RESULT_DIR" >&2
  exit 2
fi
mkdir -p "$SYNC_RESULT_DIR"
capture_postflight() {
  local status=$?
  npu-smi info > "$SYNC_RESULT_DIR/postcheck-device.txt" 2>&1 || true
  df -h "$CANN_ROOT" > "$SYNC_RESULT_DIR/postcheck-disk.txt" 2>&1 || true
  du -sh "$CANN_ROOT"/* > "$SYNC_RESULT_DIR/postcheck-project-usage.txt" 2>&1 || true
  exit "$status"
}
trap capture_postflight EXIT

npu-smi info > "$SYNC_RESULT_DIR/precheck-a-device.txt" 2>&1
df -h "$CANN_ROOT" > "$SYNC_RESULT_DIR/precheck-a-disk.txt" 2>&1
du -sh "$CANN_ROOT"/* > "$SYNC_RESULT_DIR/precheck-a-project-usage.txt" 2>&1
for rep in 1 2 3 4 5 6; do
  rep_id=$(printf '%02d' "$rep")
  "$SYNC_RUNNER" --precheck A "$rep" "$SYNC_RESULT_DIR/PRECHECK-A-$rep_id" \
    > "$SYNC_RESULT_DIR/PRECHECK-A-$rep_id-runner.log" 2>&1
done

npu-smi info > "$SYNC_RESULT_DIR/between-prechecks-device.txt" 2>&1
df -h "$CANN_ROOT" > "$SYNC_RESULT_DIR/between-prechecks-disk.txt" 2>&1
sleep 2

for rep in 1 2 3 4 5 6; do
  rep_id=$(printf '%02d' "$rep")
  "$SYNC_RUNNER" --precheck B "$rep" "$SYNC_RESULT_DIR/PRECHECK-B-$rep_id" \
    > "$SYNC_RESULT_DIR/PRECHECK-B-$rep_id-runner.log" 2>&1
done

node "$(dirname "$0")/summarize_window.mjs" "$SYNC_RESULT_DIR" \
  > "$SYNC_RESULT_DIR/window-qualification.json"
