# N05 V004 Local

LOCAL_SCORE=NONE
LOCAL_DELTA=NONE; paired medians retained as diagnostic data
LOCAL_EXECUTION=PASS
DEVICE_ID=5
RUNNER=TEMPORARY_NO_RESET_COPY_OF_EXISTING_V016_ENTRY
SAMPLE_COUNT=84 paired samples per binary across 4 cases; 21 pairs per case
LOCAL_RAW_OUTPUT=runtime-run-npu5.log
SAMPLE_PROTOCOL=warmup_each=45; order alternated P,C/C,P; DEVICE_EVENT_PRIMARY; wall timing diagnostic
CASE_FP16_TAIL=V011 median=6.280 us; V016 median=6.720 us; delta(V016-V011)=-0.020 us
CASE_FP16_WIDE=V011 median=10.040 us; V016 median=10.100 us; delta(V016-V011)=-0.160 us
CASE_BF16_TAIL=V011 median=6.920 us; V016 median=6.840 us; delta(V016-V011)=-0.040 us
CASE_BF16_WIDE=V011 median=14.100 us; V016 median=14.020 us; delta(V016-V011)=-0.360 us
RESOURCE_COMMAND=ssh cann-server3 'npu-smi info'
RESOURCE_RC=0
RESOURCE_LOG=runtime-preflight-npu5.log
NPU5_PRE_HBM=59737/65536 MB used; 5799 MB free; PID 3355594 VLLMWorker_TP 56358 MB
NPU5_POST_HBM=59880/65536 MB used; 5656 MB free; PID 3355594 VLLMWorker_TP 56354 MB
ENTRY_AUDIT=runtime-entry-audit.log
LOAD_QUALITY=VLLM_PROCESS_PRESENT
REASON=Samples completed, but VLLM load plus large outliers and high dispersion make Local invalid; score remains NONE and raw samples are preserved.
RUNTIME_FAILURE=NONE
