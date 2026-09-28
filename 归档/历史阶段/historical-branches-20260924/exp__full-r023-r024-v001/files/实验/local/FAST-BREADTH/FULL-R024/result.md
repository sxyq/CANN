# FULL-R024-CPU-VALIDATION-MATRIX V001

| 字段 | 结果 |
| --- | --- |
| route | `FULL-R024-CPU-VALIDATION-MATRIX` |
| version | `V001` |
| candidate commit | `54b4fd5` |
| source | `提交/单方案/FULL-R024-CPU-VALIDATION-MATRIX/V001/kernel.txt` |
| source SHA-256 | `3e4e9c448e8e850ba441472c281a8125d77046e40fed7f2b49ff59f75a34eaa7` |
| parent baseline | `754affb` |
| CANN environment | server3, CANN 9.0 toolchain, `dav-2201` |
| compile | PASS, config + link PASS |
| compile seconds | `11` |
| CPU matrix | PASS, `720/720` cases |
| minimum run | UNKNOWN: all 8 NPU devices had pre-existing model processes |
| run seconds | `UNKNOWN` |
| online ready | YES, compile-pass representative candidate; parent lane must apply submission confirmation policy |
| submission | not performed by this lane |

Evidence:

- CPU runner: `实验/local/FAST-BREADTH/FULL-R024/cpu_validation_matrix.py`
- CPU JSON: `实验/local/FAST-BREADTH/FULL-R024/cpu_validation_matrix.json`
- Remote source: `/home/data4t2/lelinfeng/cann/fast/lane-f/R024/kernel.asc`
- Remote compile summary: `/home/data4t2/lelinfeng/cann/fast/lane-f/R024/compile-summary.log`
- Remote build logs: `/home/data4t2/lelinfeng/cann/fast/lane-f/R024/compile-config.log` and `compile-build.log`
- Resource snapshot: `/home/data4t2/lelinfeng/cann` had `82G` available; NPU 0-7 were occupied by existing processes.

The CPU matrix is independent evidence for the mathematical chain. It is never copied to device output and does not perform the submitted kernel's core computation on Host.
