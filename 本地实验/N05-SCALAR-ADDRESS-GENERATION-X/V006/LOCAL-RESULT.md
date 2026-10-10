# N05 V006 Local

LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
LOCAL_EXECUTION=PASS
DEVICE_ID=5
RUNNER=TEMPORARY_NO_RESET_COPY_OF_EXISTING_V016_ENTRY
FORMAL_RUNNER_CREATED=NO
SAMPLE_COUNT=84 paired samples per binary across 4 cases; 21 pairs per case
LOCAL_RAW_OUTPUT=runtime-run-npu5.log
SAMPLE_PROTOCOL=warmup_each=45; order alternated P,C/C,P; DEVICE_EVENT_PRIMARY; wall timing diagnostic
CASE_FP16_TAIL=V011 median=5.980 us; V016 median=6.080 us; delta(V016-V011)=0.040 us
CASE_FP16_WIDE=V011 median=10.180 us; V016 median=9.840 us; delta(V016-V011)=-0.160 us
CASE_BF16_TAIL=V011 median=6.840 us; V016 median=6.840 us; delta(V016-V011)=-0.060 us
CASE_BF16_WIDE=V011 median=13.900 us; V016 median=13.900 us; delta(V016-V011)=-0.040 us
RESOURCE_COMMAND=ssh cann-server3 'npu-smi info'
RESOURCE_RC=0
RESOURCE_LOG=runtime-run-npu5.log
NPU5_PRE_HBM=59877/65536 MB used; 5659 MB free; PID 3355594 VLLMWorker_TP 56354 MB
NPU5_POST_HBM=59879/65536 MB used; 5657 MB free; PID 3355594 VLLMWorker_TP 56354 MB
ENTRY_AUDIT=runtime-entry-audit.log
LOAD_QUALITY=VLLM_PROCESS_PRESENT
REASON=Samples completed, but VLLM load plus large outliers and high dispersion make Local invalid; score remains NONE and raw samples are preserved.
RUNTIME_FAILURE=NONE
