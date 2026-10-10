# N05 V004 Result

ROUTE=N05-SCALAR-ADDRESS-GENERATION-X
REVISION=V004
PARENT=V003
CHANGE=Replace ProcessWideLowPrecision pass-1/pass-2 address arithmetic with the executable WideLowPrecisionCursor structure; preserve tile order, GM offsets, arithmetic, DMA, events, and row/core ownership.
SOURCE_REVIEW=PASS
COMPILE=PASS
CONFIGURE_RC=0
BUILD_RC=0
FULL_LINK=PASS
CORRECTNESS=PASS
CORRECTNESS_CASES=4; V011 and V016 failures=0 in every case
CORRECTNESS_RAW_OUTPUT=runtime-run-npu5.log
LOCAL_EXECUTION=PASS
LOCAL_SCORE=NONE
DEVICE_ID=5
RUNTIME_EXECUTION=PASS
RUNNER=TEMPORARY_NO_RESET_COPY_OF_EXISTING_V016_ENTRY
RESOURCE_STATUS=VLLM PID 3355594 on NPU5; pre 59737/65536 MB, post 59880/65536 MB
RUNTIME_ENTRY_AUDIT=runtime-entry-audit.log
CANDIDATE_SOURCE_SHA256=c3b936548900cdf623887d73f1a816e145d751f43b330011c9eee9e61837a74e
COMPILE_INPUT_ALIAS_SHA256=c3b936548900cdf623887d73f1a816e145d751f43b330011c9eee9e61837a74e
SOURCE_INPUT_MATCH=PASS
LOCAL_RAW_OUTPUT=runtime-run-npu5.log; score NONE because of load and sample outliers/high dispersion
RUNTIME_FAILURE=NONE; no OOM, runtime error, or device reset
PUSH=NO

The existing V016 CMake entry built `device`, `submission`, and `full_link` successfully. Complete command output and pre/post resource snapshots are in `compile-attempt-01.log`; artifact sizes and SHA256 values are in `build-artifacts.txt`.

The V004 source is a substantive change from V003. `WideLowPrecisionCursor` drives both address-generation paths through `Advance()`. Vector instruction scheduling, cache hints, arithmetic, DMA calls, event operations, tile traversal, and row/core ownership remain unchanged. The existing V016 paired entry was copied temporarily, changed to device 5, and had only the device-reset call removed; the paired protocol stayed unchanged. Correctness passed for all four cases. Local execution completed, while its score remains `NONE` because VLLM load and large sample outliers/high dispersion make the timing result unusable.
