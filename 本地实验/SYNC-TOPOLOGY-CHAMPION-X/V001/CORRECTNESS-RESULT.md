# SYNC-TOPOLOGY-CHAMPION-X V001 Correctness Result

Status: `CORRECTNESS=PASS`; `PERFORMANCE=NOT_RUN`.

## Source and executable identity

- Route/revision: `SYNC-TOPOLOGY-CHAMPION-X V001`
- Candidate source commit: `8e70939563297df675fe46d3045524780dd29c80`
- Candidate path: `本地实验/SYNC-TOPOLOGY-CHAMPION-X/V001/submission.asc`
- Candidate SHA256: `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`
- Build-fix commit: `29a6f65df278bb44414d5526264d95cc18833bd0`
- RUN_ID: `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-20261002T153544Z`
- Executable SHA256: `9e9bb3d578b7d967e2e11538c9dafbac50e3200764007fa1b6a4db4ae5a829f8`
- Executable path: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/build-SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-20261002T153544Z/sync_topology_v001_correctness`

## Command and result

```bash
LD_LIBRARY_PATH="$ASCEND_HOME_PATH/aarch64-linux/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
  "$BUILD_DIR/sync_topology_v001_correctness"
# RC=0; runner summary reported ALL_PASS
```

| Case | Shape | Dtype | Matched ratio | Maximum absolute error | Result |
|---|---|---|---:|---:|---|
| `h2_primary_wide_path` | `[2,12288]` | FP16 | `1.00000000` | `0.00048828125` | PASS |
| `h2_unmodified_path_control` | `[2,8192]` | FP16 | `1.00000000` | `0.00048828125` | PASS |

Runner summary: `device=4 cases=2 failures=0 correctness=ALL_PASS timing=NOT_RUN`.

## Device evidence

- Pre-run snapshot: `2026-10-02T15:42:51Z`; device 4 HBM used `61221/65536 MB`, free `4315 MB`; AICore `0%`.
- Post-run snapshot: `2026-10-02T15:43:01Z`; device 4 HBM used `61223/65536 MB`, free `4313 MB`; AICore `0%`.
- Python PID `1819590` and `VLLMEngineCor` PID `2999855` remained active; neither was changed.
- Available disk remained `628G`.
- No timing command ran. Stop here for Main to record the device 4 timing lease.

## Server logs

- Correctness log: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/logs/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-20261002T153544Z-correctness-device4.log`
- Pre-run resources: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/logs/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-20261002T153544Z-pre-correctness-resource.log`
- Post-run resources: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/logs/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-20261002T153544Z-post-correctness-resource.log`
- Executable identity: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/logs/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-20261002T153544Z-executable-identity.log`
