# N05 V002 Correctness

CORRECTNESS=NONE
DEVICE_ID=NONE
RUNTIME_PROBE=NOT_RUN
CORRECTNESS_RAW_OUTPUT=NONE
RUNNER_STARTED=NO

NPU 2 and NPU 3 had no running processes in the compile snapshot but were reserved for N01 and N02. NPU 4, NPU 5, NPU 6, and NPU 7 had other user processes. NPU 0 and NPU 1 also had existing processes. No device was used for runtime correctness, and no existing process was touched.

The raw resource and process snapshots are in `compile-attempt-02.log`.
