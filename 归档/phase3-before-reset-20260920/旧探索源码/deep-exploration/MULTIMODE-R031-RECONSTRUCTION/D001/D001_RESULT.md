# D001 compile result

- Track: `MULTIMODE-R031-RECONSTRUCTION`
- Candidate: `D001`
- Hypothesis: runtime selection by dtype, D, aligned row width, and estimated rows per Vector Core; small and medium workloads reuse parameters, wide workloads use bounded 4096-element tiles, and every dtype uses the FP32 two-pass arithmetic chain.
- Final git commit: `67d6172`
- Compile: `PASS`
- Compiler target: `dav-2201`
- Compile log: `/home/data4t2/lelinfeng/cann/实验/deep-exploration/MULTIMODE-R031-RECONSTRUCTION/D001/日志/compile.log`
- Server source: `/home/data4t2/lelinfeng/cann/实验/deep-exploration/MULTIMODE-R031-RECONSTRUCTION/D001/kernel.asc`
- Local source: `/Users/sunyiyang/Desktop/Project/cann/实验/deep-exploration/MULTIMODE-R031-RECONSTRUCTION/D001/kernel.txt`
- ONLINE_READY: `YES`

This candidate was only compiled. No local NPU correctness run, CPU matrix, profiling, benchmark, or platform submission was performed.
