# W5-R02 real w5ctl backend validation receipt

AGENT_ID=W5-R02-REGISTER-SPILL-ELIMINATION-X
ROUTE=W5-R02-REGISTER-SPILL-ELIMINATION-X
WORKTREE=/home/data4t2/lelinfeng/cann-w5-r02-register
BRANCH=research/w5-r02-register-spill
RECEIPT_TIME_UTC=2026-10-10T08:51:27Z
CURRENT_STAGE=ORTHOGONAL_RESEARCH_ONLY
REVISION=NONE

## Fixed execution boundary

No new Agent, Worktree, Runner, scorer, parallel execution chain, Candidate,
V001, donor copy, shared-tooling change, or resource probe was created. The
R31B V011 Parent was used only for read-only identity/SHA audit and was not
used as a donor.

## Environment and load

TOOLKIT_SYMLINK=/usr/local/Ascend/ascend-toolkit/latest
TOOLKIT_REAL=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
INHERITED_ASCEND_HOME_PATH=NOT_SET
INHERITED_BISHENG=NOT_IN_PATH
INHERITED_CCEC=NOT_IN_PATH
DEVICE=3
SOC=Ascend910B3
HBM_CAPACITY_MB=65536
PRE_HBM_USAGE_MB=17615
PRE_HBM_FREE_MB=47921
PRE_AICORE_PERCENT=12
PRE_EXTERNAL_LOAD=python3.11 pid 1859841; 14238 MB reported by npu-smi
POST_HBM_USAGE_RATE_PERCENT=26
POST_AICORE_PERCENT=12
POST_AIVECTOR_PERCENT=2
POST_HBM_BANDWIDTH_PERCENT=9
HBM_GATE=PASS_GT_100MB
EXTERNAL_LOAD_POLICY=RECORDED_ONLY; NOT_A_STOP_CONDITION

The pre snapshot was from `npu-smi info` immediately before the w5ctl
validation. The post snapshot was from:

```text
npu-smi info -t usages -i 3
```

## Historical backend audit

The real read-only doctor was invoked with these existing executable entries:

```text
BUILD_BACKEND=/home/data4t2/lelinfeng/cann-w5-r02-register/本地实验/REDUCE-INVSCALE-X/V002/support/compile.sh
CORRECTNESS_BACKEND=/home/data4t2/lelinfeng/cann-w5-r02-register/本地实验/REDUCE-INVSCALE-X/V002/support/run_correctness.sh
LOCAL_BACKEND=/home/data4t2/lelinfeng/cann-w5-r02-register/本地实验/REDUCE-INVSCALE-X/V002/support/run_probes.sh
RECORD_BACKEND=NOT_CONFIGURED
```

Doctor result:

```text
REPO_ROOT_CHECK=PASS
BRANCH_CHECK=PASS
WORKTREE_CHANGED_COUNT=0
WORKTREE_SCOPE=PASS
CHAMPION_CHECK=PASS
CHAMPION_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
BUILD_BACKEND=READY
CORRECTNESS_BACKEND=READY
LOCAL_BACKEND=READY
RECORD_BACKEND=NOT_CONFIGURED
NO_SECOND_RUNNER=YES
NO_SECOND_SCORER=YES
REVISION_AUTOCREATE=DISABLED
ONLINE=NOT_SUBMITTED
EXECUTION_READINESS=BLOCKED
DOCTOR_STATUS=PASS
BACKEND_BLOCKER=MISSING=RECORD_BACKEND
```

The historical Build script writes its own `build/` and `compile.log`; the
historical Correctness script writes its own `results/` and executes its own
Parent/Candidate probes. Neither was delegated because that would write or
execute another route's workflow and would not validate an R2 Candidate.

## Real w5ctl Build attempt

The command used the real historical Build backend path but an intentionally
absent R2 Candidate path. This caused w5ctl identity preflight to stop before
delegation; it was not a fake backend or a PASS.

```text
COMMAND=python3 -B 研究/REGISTER-SPILL-ELIMINATION-X/w5ctl build --repo-root /home/data4t2/lelinfeng/cann-w5-r02-register --expected-branch research/w5-r02-register-spill --route W5-R02-REGISTER-SPILL-ELIMINATION-X --route-root /home/data4t2/lelinfeng/cann-w5-r02-register/研究/REGISTER-SPILL-ELIMINATION-X --source /home/data4t2/lelinfeng/cann-w5-r02-register/研究/REGISTER-SPILL-ELIMINATION-X/Candidate/no-v001.asc --artifact /home/data4t2/lelinfeng/cann-w5-r02-register/研究/REGISTER-SPILL-ELIMINATION-X/artifacts/no-v001.o --backend /home/data4t2/lelinfeng/cann-w5-r02-register/本地实验/REDUCE-INVSCALE-X/V002/support/compile.sh --backend-workdir /home/data4t2/lelinfeng/cann-w5-r02-register/研究/REGISTER-SPILL-ELIMINATION-X --log /home/data4t2/lelinfeng/cann-w5-r02-register/研究/REGISTER-SPILL-ELIMINATION-X/build-backend-validation.log
RC=65
STATUS=BLOCKED
IDENTITY_STATUS=FAIL
BLOCKER=SOURCE_MISSING:.../研究/REGISTER-SPILL-ELIMINATION-X/Candidate/no-v001.asc
BACKEND_INVOKED=NO
OUTPUT_LOG_CREATED=NO
ARTIFACT_SHA256=NONE
```

## Real w5ctl Correctness attempt

The command used the real historical Correctness backend path and the same
absent R2 Candidate/Artifact identity. It stopped before delegation for the
same identity reason.

```text
COMMAND=python3 -B 研究/REGISTER-SPILL-ELIMINATION-X/w5ctl correctness --repo-root /home/data4t2/lelinfeng/cann-w5-r02-register --expected-branch research/w5-r02-register-spill --route W5-R02-REGISTER-SPILL-ELIMINATION-X --route-root /home/data4t2/lelinfeng/cann-w5-r02-register/研究/REGISTER-SPILL-ELIMINATION-X --source /home/data4t2/lelinfeng/cann-w5-r02-register/研究/REGISTER-SPILL-ELIMINATION-X/Candidate/no-v001.asc --artifact /home/data4t2/lelinfeng/cann-w5-r02-register/研究/REGISTER-SPILL-ELIMINATION-X/artifacts/no-v001.o --backend /home/data4t2/lelinfeng/cann-w5-r02-register/本地实验/REDUCE-INVSCALE-X/V002/support/run_correctness.sh --backend-workdir /home/data4t2/lelinfeng/cann-w5-r02-register/研究/REGISTER-SPILL-ELIMINATION-X --log /home/data4t2/lelinfeng/cann-w5-r02-register/研究/REGISTER-SPILL-ELIMINATION-X/correctness-backend-validation.log
RC=65
STATUS=BLOCKED
IDENTITY_STATUS=FAIL
BLOCKER=SOURCE_MISSING:.../研究/REGISTER-SPILL-ELIMINATION-X/Candidate/no-v001.asc
BACKEND_INVOKED=NO
OUTPUT_LOG_CREATED=NO
ARTIFACT_SHA256=NONE
```

## Identity, raw, and resource status

PARENT=/home/data4t2/lelinfeng/cann-w5-r02-register/线上结果/R31B/V011/submission.asc
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_USE=READ_ONLY_IDENTITY_AUDIT; NOT_DONOR
CANDIDATE=NONE
CANDIDATE_SHA256=NONE
ARTIFACT=NONE
ARTIFACT_SHA256=NONE
RAW_RECORD=NONE
LOCAL=NOT_RUN
POLLUTION=NO_NEW_ROUTE_OR_OTHER_ROUTE_WRITES
RESOURCE_EVIDENCE=NO_NEW_DIRECT_REGISTER_SPILL_SCRATCH_LIVE_RANGE_FIELDS
HYPOTHESIS_STATUS=FEASIBILITY_UNPROVEN
CANDIDATE_EDIT=NO
V001_CREATED=NO
CORRECTNESS=NOT_RUN
ONLINE=NOT_SUBMITTED

The existing compiler receipts remain unchanged: no direct register-pressure,
spill/reload, scratch, or live-range evidence exists. No resource probe was
repeated.

## Route receipt

LAST_ACTION=REAL_W5CTL_DOCTOR_AND_PREDELEGATION_BUILD_CORRECTNESS_RC65
BUILD=BLOCKED_PREDELEGATION_RC65
CORRECTNESS=BLOCKED_PREDELEGATION_RC65
NEXT_ACTION=STOP; REQUIRE_DIRECT_RESOURCE_EVIDENCE_OR_AN_R2_APPROVED_BACKEND
BLOCKER=NO_R2_CANDIDATE_OR_ARTIFACT; HISTORICAL_BACKENDS_ARE_NOT_R2_SCOPED
COMMIT=SEE_COMMIT_CONTAINING_THIS_RECEIPT
REMOTE_SHA=VERIFY_AFTER_PUSH
