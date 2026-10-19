# N04 V005 Handoff

V005 uses V004 as Parent and changes only the existing output-path cache mode from `CACHE_MODE_NORMAL` to `CACHE_MODE_PERSISTENT`. The input disable Hint remains unchanged. Candidate source commit is `4d10c4113ce8fb5a6cff52d451f5d77c8f48d077`, source SHA256 is `e436967292ce60c958da592f8e62106244b54b8e82169234c7a85f46494c36a9`, and the paired harness mapping is `5cc43671434d16050539cee4e60f4e2ca3588e19`.

Compile and paired build passed. Correctness passed 4/4. Local completed 21 alternating pairs for each public case after 45 warmups. The Parent/Candidate medians, deltas, MAD, CV, raw evidence, d1 load, and `LOCAL_SCORE=NONE` reasoning are in `RESULTS.md` and `local-paired.log`.

Main's ten FULL_SOURCE_SIMILARITY values around `0.982-0.999` are retained as a real shared-Champion fact. No cosmetic source changes were made to lower them, and effective-change similarity remains for the separate audit.

The remote run directory is `~/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V005/4d10c411`. No V005 process remains. Future work must stay on the existing executable GM/L2 axis without adding a Hint or changing DMA, tiling, arithmetic, or parameter residency.
