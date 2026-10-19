# N04 V003 Handoff

V003 uses V002 as Parent and changes only the output-path cache mode from `CACHE_MODE_DISABLE` to `CACHE_MODE_LAST`. The V001 input disable hint remains in the Kernel. Candidate source commit is `6c53cf0e9410825cfae2edf38b7da1502b3afa9e`, source SHA256 is `c00d8d4d5ea1c16636c1df1f0c541f42a4bdbde1e4c21c4f434e723e2cabb6db`, and the existing paired harness version mapping is `4857b562`.

Adapter/device and paired builds completed after the existing seed dependency was placed in the support directory. Correctness passed 4/4. Paired Local completed 21 pairs for each of the four public cases with 45 warmups; raw output is in `local-paired.log`. The deltas were noisy under the existing d1 workload, so the result is `LOCAL_SCORE=NONE`.

The remote run directory is `~/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V003/6c53cf0e`. No N04 process remains. Keep all V003 raw evidence and use V003 as the source Parent for the next single-factor revision.
