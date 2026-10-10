# N02 V002 Revision Declaration

ROUTE=N02 CODE-FOOTPRINT-ICACHE-X
REVISION=V002
PARENT_REVISION=V001
PARENT_COMMIT=031fcc5351b25eafa4f6ce04bd426f3f487fdb7b
PARENT_SOURCE=本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V001/submission.asc
PARENT_SOURCE_SHA256=2238d0ff3604f0de752d9d9ecb5546f0ec94aa2205b9018853cdd19d7ca04fd1
REFERENCE_PARENT_SOURCE=线上结果/R31B/V011/submission.asc
REFERENCE_PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CHANGE=Use the existing paired_runner_v016 target with explicit independent source-directory cache values: R31B_V011_SOURCE_DIR points to the V011 Parent input and R31B_V016_SOURCE_DIR points to the N02 Candidate input.
CONFIG_SCOPE=Existing paired runner path configuration only; no kernel source change, no new Runner, no R029 mode dispatch.
SHAPE_PATH=UNCHANGED
ALGORITHM=UNCHANGED
MECHANISM=CODE-FOOTPRINT-ICACHE-X input selection for paired compilation
FORBIDDEN_OVERLAP=R029 mode dispatch; N01 register lifetime; N03 vector ILP; N04 GM/L2 access; N05 scalar address generation.
REMOTE_TEMP=/tmp/cann-n02-code-footprint-icache-x-v002-y3fwWq
DEVICE=3
DEVICE_REASON=User snapshot reported NPU3 without a running process and about 3429 MB HBM used; other devices are not touched.
RUNNER_SAFETY=Existing paired_runner_v016 hardcodes kDevice=4 and calls aclrtResetDevice(4) on exit; compile is allowed, runtime execution is forbidden for this Revision.
RULE_REFRESH_RECEIPT=ROUTE=N02 CODE-FOOTPRINT-ICACHE-X; HEAD=031fcc5351b25eafa4f6ce04bd426f3f487fdb7b; PARENT=130278f73e8ccc292b102a81985e9dfd952d7a07; DIRECT_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f; MECHANISM=hotspot helper layout, inline/noinline, and instruction-fetch footprint; FORBIDDEN_OVERLAP=R029 mode dispatch, N01 register lifetime, N03 vector ILP, N04 GM/L2 access, N05 scalar address generation; NEXT_ACTION=compile existing paired_runner_v016 with separate Parent/Candidate source paths.
COMPILE=PASS_AFTER_ENV_FIX
COMPILE_ATTEMPTS=First attempt failed with BUILD_RC=2 because the ASC plugin could not find <vector>; second attempt set the existing V016 CPLUS_INCLUDE_PATH and system-first LD_LIBRARY_PATH and returned CONFIG_RC=0 BUILD_RC=0.
COMPILE_COMMAND=cmake -S /tmp/cann-n02-code-footprint-icache-x-v002-y3fwWq/paired -B /tmp/cann-n02-code-footprint-icache-x-v002-y3fwWq/paired/build -DR31B_V011_SOURCE_DIR=/tmp/cann-n02-code-footprint-icache-x-v002-y3fwWq/artifact-parent -DR31B_V016_SOURCE_DIR=/tmp/cann-n02-code-footprint-icache-x-v002-y3fwWq/artifact-candidate; cmake --build /tmp/cann-n02-code-footprint-icache-x-v002-y3fwWq/paired/build --target paired_runner_v016 --parallel 1 -- -B
COMPILE_INPUT=Dependency file resolved Parent=/tmp/cann-n02-code-footprint-icache-x-v002-y3fwWq/artifact-parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc and Candidate=/tmp/cann-n02-code-footprint-icache-x-v002-y3fwWq/artifact-candidate/R31B-V016-WIDE-TILE-SEED_kernel.asc.
GENERATED_DIFFERENCE=Path configuration and dependency paths are corrected; runner size is 625984 bytes and runner SHA256 remains 9a0966dd23c716927b73d9eafd21d993a477c2c2ed855ea2a1032681d87cf43b, equal to the V001 runner because V001 independently established equal kernel codegen.
CORRECTNESS=NOT_RUN
CORRECTNESS_REASON=NPU3 was idle with no process, but the existing paired runner hardcodes device 4 and calls aclrtResetDevice(4) on exit. No runtime command was issued.
LOCAL=NONE
LOCAL_REASON=No safe runtime entry was available; no device-event sample was collected.
RESULT=PAIRED_INPUT_PATH_FIXED; CODEGEN_EQUAL_RECONFIRMED; NO_RUNTIME_RESULT
GIT_COMMIT=TO_BE_RECORDED
EVIDENCE=logs/v002-preflight-compile-device3.log; logs/v002-envfix-compile-device3.log; logs/v002-result.log
NEXT_ACTION=Keep V002 as compile-only path correction; a later N02 source revision must change executable code before any performance interpretation.
