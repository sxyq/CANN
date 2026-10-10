# W5-R01 CROSSROW-EPI-PROLOGUE-X V003 shape-level Local diagnostic

AGENT_ID: W5-R01/Zeno
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V003
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
STAGE: LOCAL_PREFLIGHT
HEAD_AT_PREFLIGHT: b90198271699bc116fa631ad0bf6326c2533014f

## Registered identity

PARENT_REVISION: V002 Candidate (snapshotted as V003/Parent.asc)
PARENT_SOURCE_SHA256: `426489bfdc63441f2fd179fdff176875b24d815e6e0420343aeed0b0dab7933f`
CANDIDATE_SOURCE_SHA256: `c6d8da88a9ba03e6c730733c52bed051b54ebc94629196636ea415599b96eb8c`
RUNNER_PATH: `本地实验/CROSSROW-EPI-PROLOGUE-X/V001/support/build/crossrow_v001_paired_runner`
RUNNER_SHA256: `5210262b45c50878c24def38e108386b45bd017086a61a1293ab290d171a4d5a`
ADAPTER_STATUS: V001 paths restored; no temporary V003 selector was installed because lease qualification failed.

## Registered shape and protocol

SHAPE: `rows=16,width=6144,blocks=8,dtype=fp16`
TIMING_PROTOCOL: `45 warmups, 21 paired samples, alternating PC/CP, device-event`
SCOPE: shape-level diagnostic only; this is not a complete Local score.

## Read-only device and lease preflight

HOSTNAME: `hwnput3`
DEVICE_COMMAND: `npu-smi info`
TARGET_CANDIDATES: NPU2 and NPU3 by numeric HBM gate only
NPU2_PRE: `HBM 63350/65536 MB used; FREE_HBM approximately 2186 MB; AICore 66%; PID 658669 python, 59977 MB`
NPU3_PRE: `HBM 63352/65536 MB used; FREE_HBM approximately 2184 MB; AICore 71%; PID 658670 python, 59977 MB`
OTHER_DEVICES: occupied processes also present and retained in the live `npu-smi info` snapshot.
HBM_GATE: `NUMERIC_PASS (>100 MB)`
LEASE_STATUS: `NO_CURRENT_ROUTE_OWNED_AUTHORIZED_LEASE`
LEASE_OWNER: `UNVERIFIED`
ROUTE_LEASE_FILES: only historical `V002/lease-blocker-20261009.log` and `V002/server3-lease-check.log`; no current authorization/owner record
LEASE_DECISION: HBM headroom does not establish authorization; no device was claimed.

## Measurement result

LOCAL_COMMAND: `NOT_RUN`
LOCAL_RC: `NOT_APPLICABLE`
RAW_PATH: `NOT_CREATED; no samples were generated`
MEDIAN: `NOT_AVAILABLE`
PAIRED_DELTA: `NOT_AVAILABLE`
LOAD_QUALITY: `MEASUREMENT_BLOCKED_NO_AUTHORIZED_LEASE`
LOCAL_VERDICT: `MEASUREMENT_BLOCKED`
SHAPE_LOCAL_ONLY: `YES`
FULL_LOCAL_SCORE: `NOT_COMPUTABLE`
LOCAL_BEST: `UNCHANGED`
NO_V002_RERUN: `YES`
NO_KERNEL_OR_IDENTITY_CHANGE: `YES`

## Next action

Obtain a current authorized lease/owner for a non-conflicting device, then perform exactly one V003 shape-level paired diagnostic using the existing Runner and the registered identity above. Until that evidence exists, do not run Local, do not claim `LOCAL_BEST` or `COMPLETE`, and do not alter V002.
