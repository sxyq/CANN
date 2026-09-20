# MIX-A D001 Result

- Track: `MIX-A-REDUCE-FP32-TWOPASS`
- Candidate: `D001`
- Hypothesis: 2048-element FP32 reduction tiles with row-wise scheduling and a two-pass reread keep the reduction workspace bounded while avoiding a global y buffer.
- Local source: `提交/混合方案/H002-性能组合/MIX-A-REDUCE-FP32-TWOPASS/D001/kernel.txt`
- Server source: `/home/data4t2/lelinfeng/cann/源码/MIX-A-REDUCE-FP32-TWOPASS/D001/kernel.asc`
- Final commit: `03b48db`
- Compile: `PASS`
- Compile log: `/home/data4t2/lelinfeng/cann/实验/MIX-A-REDUCE-FP32-TWOPASS/D001/日志/compile.log`
- Toolchain: CANN 9.0 project toolchain, target `dav-2201`, server SoC `Ascend 910B3`
- Runtime validation: not executed by task scope
- Profiling/performance comparison: not executed by task scope
