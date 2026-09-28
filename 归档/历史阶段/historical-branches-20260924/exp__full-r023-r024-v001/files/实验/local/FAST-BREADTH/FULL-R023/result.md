# FULL-R023-ENGINEERING-SPEC V001

| 字段 | 结果 |
| --- | --- |
| route | `FULL-R023-ENGINEERING-SPEC` |
| version | `V001` |
| candidate commit | `54b4fd5` |
| source | `提交/单方案/FULL-R023-ENGINEERING-SPEC/V001/kernel.txt` |
| source SHA-256 | `fab032d8e2baedc20777adc843e1b35e39e4eafa15502edf5968e36e3aeceafc` |
| parent baseline | `754affb` |
| CANN environment | server3, CANN 9.0 toolchain, `dav-2201` |
| compile | PASS, config + link PASS |
| compile seconds | `11` |
| minimum run | UNKNOWN: all 8 NPU devices had pre-existing model processes |
| run seconds | `UNKNOWN` |
| online ready | YES, compile-pass representative candidate; parent lane must apply submission confirmation policy |
| submission | not performed by this lane |

Evidence:

- Remote source: `/home/data4t2/lelinfeng/cann/fast/lane-f/R023/kernel.asc`
- Remote compile summary: `/home/data4t2/lelinfeng/cann/fast/lane-f/R023/compile-summary.log`
- Remote build logs: `/home/data4t2/lelinfeng/cann/fast/lane-f/R023/compile-config.log` and `compile-build.log`
- Static checklist: `实验/local/FAST-BREADTH/FULL-R023/static-validation.md`
- Resource snapshot: `/home/data4t2/lelinfeng/cann` had `82G` available; NPU 0-7 were occupied by existing processes.

The route claim is represented in the submitted kernel by strict direct-invocation validation, 64-bit address arithmetic, explicit synchronization helpers, and valid-byte tail copies. Core arithmetic remains on device.
