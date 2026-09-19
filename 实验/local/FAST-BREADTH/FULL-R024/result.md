# FULL-R024-CPU-VALIDATION-MATRIX V001

| 字段 | 结果 |
| --- | --- |
| route | `FULL-R024-CPU-VALIDATION-MATRIX` |
| version | `V001` |
| candidate commit | `6a39489` |
| CPU matrix commit | `103ab8e` |
| compile evidence commit | pending |
| source | `提交/单方案/FULL-R024-CPU-VALIDATION-MATRIX/V001/kernel.txt` |
| source SHA-256 | `7f4be21082e5aeb9b82b2b80471824673c1352383be90f30354c5904c1d26118` |
| CPU matrix | `1080/1080` |
| transfer coverage | data aligned `10482`, data padded `17934`, parameter aligned `15540`, parameter padded `12876` |
| CANN environment | server3, CANN 9.0 project toolchain, `dav-2201`, Ascend 910B3 |
| configure | PASS, `0.30s` |
| compile | PASS, config and link PASS, `11.72s` |
| minimum run | NOT RUN: all 8 NPU devices had pre-existing model processes |
| run seconds | `UNKNOWN` |
| CANNJudge | not performed |

Evidence paths:

- CPU runner: `实验/local/FAST-BREADTH/FULL-R024/cpu_validation_matrix.py`
- CPU JSON: `实验/local/FAST-BREADTH/FULL-R024/cpu_validation_matrix.json`
- Remote source: `/home/data4t2/lelinfeng/cann/fast/full-r024-cpu-validation-matrix-v001/R024/kernel.asc`
- Remote binary: `/home/data4t2/lelinfeng/cann/fast/full-r024-cpu-validation-matrix-v001/R024/build/add_rms_norm_bias_custom`
- Remote configure log: `/home/data4t2/lelinfeng/cann/fast/full-r024-cpu-validation-matrix-v001/R024/compile-config.log`
- Remote build log: `/home/data4t2/lelinfeng/cann/fast/full-r024-cpu-validation-matrix-v001/R024/compile-build.log`
- Remote resource record: `/home/data4t2/lelinfeng/cann/fast/full-r024-cpu-validation-matrix-v001/R024/resource-before.txt`

The CPU matrix is independent evidence for the mathematical chain and transfer plan. It does not supply data to the device output.
