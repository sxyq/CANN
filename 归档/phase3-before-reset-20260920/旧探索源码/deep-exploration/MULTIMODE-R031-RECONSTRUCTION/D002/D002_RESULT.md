# D002 compile result

- Track: `MULTIMODE-R031-RECONSTRUCTION`
- Candidate: `D002`
- Hypothesis: use `avgRows = floor(rowCount / blockCount)` for batch-mode boundaries; enable multi-row parameter reuse only when the average rows per Vector Core reaches the plan threshold, and otherwise use the single-row plan.
- Final git commit before this record: `7300b6b`
- Compile: `PASS`
- Compiler target: `dav-2201`
- Compile log: `/home/data4t2/lelinfeng/cann/实验/deep-exploration/MULTIMODE-R031-RECONSTRUCTION/D002/日志/compile.log`
- Server source: `/home/data4t2/lelinfeng/cann/实验/deep-exploration/MULTIMODE-R031-RECONSTRUCTION/D002/kernel.asc`
- Local source: `/Users/sunyiyang/Desktop/Project/cann/实验/deep-exploration/MULTIMODE-R031-RECONSTRUCTION/D002/kernel.txt`
- ONLINE_READY: `YES`

This candidate was only compiled. No local NPU correctness run, CPU matrix, profiling, benchmark, or platform submission was performed.
