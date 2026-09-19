# FAST-BREADTH lane-A R004 V001 CompileFix-C

- Route: `FULL-R004-LOW-PRECISION-MIDDLE`
- Candidate commit: `dea39fe`
- Source: `提交/单方案/FULL-R004-LOW-PRECISION-MIDDLE/V001/kernel.txt`
- Source SHA-256: `5dca8a1c45ca98b05c3bd063026f0fad7ea334fa075da368f80175308734718e`
- Compile attempt 1: FAIL before kernel diagnostics (`CMAKE_STATUS=0`, `BUILD_STATUS=2`, `4.72s`); copied harness lacked `data_utils.h`
- Compile attempt 2: PASS (`CMAKE_STATUS=0`, `BUILD_STATUS=0`, `12.73s`); five existing `cce_global` warnings only
- Compile log: `实验/local/FAST-BREADTH/lane-a/R004/V001/compilefix-c.log`
- Environment log: `实验/local/FAST-BREADTH/lane-a/R004/V001/environment-compilefix-c-retry.log`
- Remote build: `/home/data4t2/lelinfeng/cann/实验/FAST-BREADTH/lane-a/R004/V001/compilefix-c/build-fix-c/add_rms_norm_bias_custom`
- Minimum NPU run: PENDING; all eight NPU devices were occupied by existing model tasks
- CANNJudge: NOT_TRIGGERED
- ONLINE_READY: NO
