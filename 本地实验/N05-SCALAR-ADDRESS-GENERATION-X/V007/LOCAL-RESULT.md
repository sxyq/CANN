# N05 V007 Local

LOCAL_EXECUTION=PASS
LOCAL_SCORE=NONE
DEVICE_ID=5
RUNNER=TEMPORARY_NO_RESET_COPY_OF_EXISTING_V016_ENTRY
SAMPLE_COUNT=84 pairs total; 21 pairs per case; 168 parent/candidate device samples
LOCAL_RAW_OUTPUT=runtime-run-npu5.log
SAMPLE_PROTOCOL=warmup_each=45; order alternated P,C/C,P; DEVICE_EVENT_PRIMARY; wall timing diagnostic
CASE_FP16_TAIL=V011 median=6.140 us; V016 median=6.260 us; delta(V016-V011)=0.140 us
CASE_FP16_WIDE=V011 median=10.380 us; V016 median=10.060 us; delta(V016-V011)=-0.080 us
CASE_BF16_TAIL=V011 median=6.700 us; V016 median=6.800 us; delta(V016-V011)=0.080 us
CASE_BF16_WIDE=V011 median=14.120 us; V016 median=13.920 us; delta(V016-V011)=-0.340 us
RESOURCE_COMMAND=ssh cann-server3 'npu-smi info'
RESOURCE_RC=0
NPU5_PRE_HBM=59876/65536 MB used; PID 3355594 VLLMWorker_TP 56354 MB
NPU5_POST_HBM=59878/65536 MB used; PID 3355594 VLLMWorker_TP 56354 MB
LOAD_QUALITY=VLLM_PROCESS_PRESENT
REASON=Samples completed, but VLLM load and large outliers/high dispersion make the timing result unusable; raw samples are preserved.
PUSH=NO
