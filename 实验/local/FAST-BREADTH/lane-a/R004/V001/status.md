# FAST-BREADTH lane-A R004 V001 CompileFix-C

- Route: `FULL-R004-LOW-PRECISION-MIDDLE`
- Candidate commit: `dea39fe`
- Source: `提交/单方案/FULL-R004-LOW-PRECISION-MIDDLE/V001/kernel.txt`
- Source SHA-256: `5dca8a1c45ca98b05c3bd063026f0fad7ea334fa075da368f80175308734718e`
- Compile attempt: FAIL before kernel diagnostics (`CMAKE_STATUS=0`, `BUILD_STATUS=2`, `4.72s`)
- Compile attempt failure: copied harness lacked `data_utils.h`; the source was not compiled
- Compile log: `实验/local/FAST-BREADTH/lane-a/R004/V001/compilefix-c.log`
- Minimum NPU run: NOT_STARTED
- CANNJudge: NOT_TRIGGERED
- ONLINE_READY: NO
