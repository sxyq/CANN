# N02 V001 Revision Declaration

ROUTE=N02 CODE-FOOTPRINT-ICACHE-X
REVISION=V001
PARENT_COMMIT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
PARENT_SOURCE=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SOURCE_BYTES=190020
CANDIDATE_SOURCE=本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V001/submission.asc
CANDIDATE_SOURCE_SHA256=2238d0ff3604f0de752d9d9ecb5546f0ec94aa2205b9018853cdd19d7ca04fd1
CANDIDATE_SOURCE_BYTES=190039
CHANGE=Add __attribute__((noinline)) to AddRmsNormBiasKernel<T>::Load at submission.asc:3367.
SHAPE_PATH=UNCHANGED
ALGORITHM=UNCHANGED
FORBIDDEN_OVERLAP=R029 mode dispatch; N01 register lifetime; N03 vector ILP; N04 GM/L2 access; N05 scalar address generation.
PERFORMANCE_HYPOTHESIS=Out-of-line Load reduces repeated helper instruction body at hot call sites; any call overhead is measured against possible instruction-fetch pressure reduction.
DIFF=diff.patch
RULE_REFRESH_RECEIPT=ROUTE=N02 CODE-FOOTPRINT-ICACHE-X; HEAD=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f; PARENT=c9510be4e4a9da0084433e45442468da1eb75c72; DIRECT_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f; MECHANISM=hotspot code footprint and instruction fetch; NEXT_ACTION=compile V001 on server3.
DEVICE=4
COMPILE=PARTIAL_PASS_BUT_PAIRED_INPUT_INVALID
COMPILE_DETAIL=The existing V016 single-version targets device submission full_link returned 0, but the paired build used PARENT_INCLUDE_DIR=/tmp/cann-n02-code-footprint-icache-x-v001-jziPyb/artifact-candidate and CANDIDATE_INCLUDE_DIR=/tmp/cann-n02-code-footprint-icache-x-v001-jziPyb/artifact-candidate. device.cpp included R31B-V016-WIDE-TILE-SEED_kernel.asc while the Candidate source had also been staged under R31B-V011-LP-ROW-PIPELINE_kernel.asc, so the earlier binary-equality sample is invalid for P/C comparison.
CORRECTNESS=NOT_RUN
CORRECTNESS_REASON=The existing paired runner is shared under /tmp/cann-n02-code-footprint-icache-x-v001-jziPyb/paired and calls aclrtResetDevice(4); NPU 4 had active VLLMEngineCor PID 2999855. The runner was not executed.
LOCAL=NONE
LOCAL_REASON=No valid independent Parent/Candidate runtime input; no device sample was collected.
BINARY_EQUALITY=INVALID_EVIDENCE
FAILURE_EVIDENCE=logs/v016-paired-build-device4.log; logs/v016-paired-build-device4-envfix.log; logs/compiled-input-source-audit.log; logs/compiled-input-dependency-confirm.log
GIT_COMMIT=LOCAL_COMMIT_TO_REPORT
NEXT_ACTION=STOP V001; preserve evidence and do not run a new Runner
