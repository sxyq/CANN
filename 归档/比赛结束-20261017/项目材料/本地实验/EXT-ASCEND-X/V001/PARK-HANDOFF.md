ROUTE=EXT-ASCEND-X
REVISION=V001
DECISION=LOCAL_REJECTED
PARK_RECOMMENDATION=PARK_EXPLORE_SLOT

FINAL_CANDIDATE_SHA256=bd2b18cedf25ee4cb51a63690a1e2e4f48a9fa18766674f7401aa24b56072b8a
RUNNER_SHA256=56163104edd986446b4124f6fda021b98038e13e76add0fbb44504e89572b1c2

CORRECTNESS_RESULT=27/27 FAIL
CORRECTNESS_LOG=logs/outlife_correctness_v001.log
MATRIX=FP32; D=256/4096/8192; rows=1/2/3; availableCoreNum=1/2/3

FIX_1=V001_CORRECTNESS_FIX; PIPE_ALL scalar broadcast/compute and VECOUT-to-MTE3 diagnostic; APPLIED_AND_FAILED
FIX_1_SUBMISSION_SHA256=235ea3b6ce3bccf12a254c8f95187a288737d2d848757decedf9baae68c650f6
FIX_1_BUILD_LOG=logs/server3_correctness_sync_build_v001_q.log
FIX_1_CORRECTNESS_LOG=logs/correctness_sync_diag_v001.log

FIX_2=V001_CORRECTNESS_FIX_2; scalarStride 16B to 32B alignment fix; APPLIED_AND_FAILED
FIX_2_SUBMISSION_SHA256=3d1e6bfc8fb8d3c31ddc1210d28189377dd892903bae452cc647051ef1a4b42e
FIX_2_BUILD_LOG=logs/server3_scalar_align_build_v001_r.log
FIX_2_CORRECTNESS_LOG=logs/scalar_align_correctness_v001.log

FIX_3=V001_CORRECTNESS_FIX_3; scalar/reduction PIPE_ALL dependency fix; APPLIED_AND_FAILED
FIX_3_SUBMISSION_SHA256=cec5b465122dcd387c661db2ab1b31fef717bd0ef544726649341f2c4c84b378
FIX_3_BUILD_LOG=logs/server3_reduce_sync_build_v001_s.log
FIX_3_CORRECTNESS_LOG=logs/reduce_sync_correctness_v001.log

FIX_4=V001_CORRECTNESS_FIX_4; VECOUT/DataCopyPad output lifetime fix; APPLIED_AND_FAILED
FIX_4_SUBMISSION_SHA256=bd2b18cedf25ee4cb51a63690a1e2e4f48a9fa18766674f7401aa24b56072b8a
FIX_4_BUILD_LOG=logs/server3_outlife_build_v001_t.log
FIX_4_CORRECTNESS_LOG=logs/outlife_correctness_v001.log

BUILD_HOST=hwnput3
BUILD_SOC=Ascend910B3
BUILD_COMMANDS=cmake --build build3 --target ext_device -j2; cmake --build build3 --target ext_submission -j2; cmake --build build3 --target ext_correctness -j2
BUILD_LINK_RESULT=EXT_DEVICE_EXIT=0; EXT_SUBMISSION_EXIT=0; EXT_CORRECTNESS_FULL_LINK_EXIT=0

TIMING=NOT_STARTED
ONLINE_SUBMISSION=NOT_DONE
DO_NOT_CONTINUE_FROM_FAILED_CANDIDATE=true
NEXT_REPLACEMENT=Main assigns a new Agent to take over this parked Explore slot.
NO_NEW_REVISION_CREATED=true

EVIDENCE_STATUS=COMPLETE
EVIDENCE_ROOT=phase4/local/EXT-ASCEND-X/V001
SOURCE_META=source-meta.json
SOURCE_META_STATUS=FINAL_SHA_LOCAL_REJECTED_PARK_EXPLORE_SLOT
SOURCE_META_CURRENT_SHA256=bd2b18cedf25ee4cb51a63690a1e2e4f48a9fa18766674f7401aa24b56072b8a
SOURCE_META_PARENT=DIRECT_PARENT:null; PARENT_SOURCE_SHA:null; PARENT_STATUS=NO_APPLICABLE_PARENT
SOURCE_META_HYPOTHESIS=dynamic blockFactor/rowFactor/ubFactor tiling

EVIDENCE_FILES=submission.asc; local_full.asc; submission.sha256; diff.patch; local-result.json; source-meta.json
FIX_EVIDENCE=FIX_1..FIX_4 build and correctness logs preserved under logs/
MANIFEST_CHECK=submission.asc: OK
JSON_CHECK=source-meta.json and local-result.json valid

STOP_REASON=FIX_1..FIX_4 all remained APPLIED_AND_FAILED; final FIX_4 correctness was 27/27 FAIL. The final candidate is rejected for correctness and must not be continued.
LIFECYCLE_STATUS=PARKED_PENDING_MAIN_CLOSE
MAIN_NEXT_ACTION=Main closes this parked V001 lifecycle; any replacement is assigned to a new Agent.
NO_SOURCE_OR_RUNNER_CHANGE=true
NO_DEVICE_RUN_AFTER_PARK=true
