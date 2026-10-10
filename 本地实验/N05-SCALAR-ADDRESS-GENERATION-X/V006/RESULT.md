# N05 V006 Result

ROUTE=N05-SCALAR-ADDRESS-GENERATION-X
REVISION=V006
PARENT=V004
PARENT_COMMIT=0020dafb77638d4cd4283037d240f4bce8d29eb8
CHANGE=Pass2 parameter address consumption by valid-length carry inside ProcessWideLowPrecision; preserve tile order, output addresses, arithmetic, DMA, events, synchronization, and Row/Core ownership.
SOURCE_REVIEW=PASS
CORRECTNESS=PASS
CORRECTNESS_CASES=4; V011 and V016 failures=0 in every case
CORRECTNESS_RAW_OUTPUT=runtime-run-npu5.log
COMPILE=PASS
CONFIGURE_RC=0
BUILD_RC=0
FULL_LINK=PASS
LOCAL_EXECUTION=PASS
LOCAL_SCORE=NONE
DEVICE_ID=5
RUNTIME_EXECUTION=PASS
RUNNER=TEMPORARY_NO_RESET_COPY_OF_EXISTING_V016_ENTRY
FORMAL_RUNNER_CREATED=NO
RESOURCE_STATUS=VLLM PID 3355594 on NPU5; pre 59877/65536 MB, post 59879/65536 MB
RUNTIME_ENTRY_AUDIT=runtime-entry-audit.log
CANDIDATE_REMOTE_ALIAS_SHA256=81e8b3e953321c43d4e0f4838558221497d377ddf21e6104016f6dbdaf9aa149
PARENT_REMOTE_ALIAS_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=81e8b3e953321c43d4e0f4838558221497d377ddf21e6104016f6dbdaf9aa149
LOCAL_RAW_OUTPUT=runtime-run-npu5.log; score NONE because of load and sample outliers/high dispersion
RUNTIME_FAILURE=NONE; no OOM, runtime error, or device reset
PUSH=NO

V006 has a source-audited Scalar address-generation factor distinct from V001 flat-unit arithmetic, V003 output-row carry, V004 cursor state, and the rejected V005 proposal. The existing V016 CMake entry built `device`, `submission`, and `full_link` successfully. The temporary no-reset paired copy built and ran on NPU5 with the existing four-case protocol. Correctness passed for all four cases. Local execution completed, while its score remains `NONE` because VLLM load and large sample outliers/high dispersion make the timing result unusable.
