# V001 Correctness

CORRECTNESS=NOT_RUN
DEVICE_ID=7
RUNTIME_PROBE=NOT_RUN
CORRECTNESS_RAW_OUTPUT=NONE

The existing `runtime_probe.cpp` ends by calling `aclrtResetDevice(device)`. NPU 7 had existing processes PID 439848 and PID 445986 before and after compilation. The probe was not run, so those processes and their device were left untouched. The successful CANN compile does not establish runtime correctness.
