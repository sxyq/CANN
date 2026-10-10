# N05 V007 Result

ROUTE=N05-SCALAR-ADDRESS-GENERATION-X
REVISION=V007
PARENT=V006
PARENT_COMMIT=1251982394eaf6803b34e86e4d01df4e8ba56d9d
CHANGE=Pass1 retained-y UB address stream uses a valid-span scalar offset; all GM addresses and Pass2 parameter state are unchanged.
SOURCE_AUDIT=PASS
COMPILE=PASS
COMPILE_TARGETS=device,submission,full_link
CORRECTNESS=PASS
CORRECTNESS_CASES=4; V011 and V016 failures=0 in every case
LOCAL_EXECUTION=PASS
LOCAL_SCORE=NONE
DEVICE_ID=5
RUNNER=TEMPORARY_NO_RESET_COPY_OF_EXISTING_V016_ENTRY
FORMAL_RUNNER_CREATED=NO
RUNTIME=PASS
RUNTIME_RUN_RC=0
RUNTIME_BUILD=PASS; CONFIGURE_RC=0; BUILD_RC=0
RUNTIME_ENTRY_AUDIT=runtime-entry-audit.log
RAW_COMPILE_LOG=compile-attempt-01.log
RAW_RUNTIME_LOG=runtime-run-npu5.log
RAW_SAMPLES=84 pairs total; 21 pairs per case; 4 cases; 168 parent/candidate device samples
NPU5_PRE_HBM=59876/65536 MB used; PID 3355594 VLLMWorker_TP 56354 MB
NPU5_POST_HBM=59878/65536 MB used; PID 3355594 VLLMWorker_TP 56354 MB
LOCAL_REASON=VLLM load and high outlier dispersion; LOCAL_SCORE=NONE
RUNTIME_FAILURE=NONE; no OOM or runtime error
RESET_DEVICE_EXECUTION=NONE
REMOTE_TEMP_CLEANUP_RC=0
PUSH=NO

V007 retains the substantive Pass1 retained-y local-address change. Standalone Compile and the existing no-reset paired runtime both passed; Correctness passed all four cases. Local completed but remains NONE because the preserved paired samples are strongly affected by VLLM load and outliers.
