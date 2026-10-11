# N04 V002 Results

## Source

- Route: N04-GM-CACHE-STREAMING-X
- Branch: `route/n04-gm-cache-streaming-x-20261011`
- V001 source parent commit: `e498788ae5a577ff36970f27e4a8cab2cea45245`
- Candidate commit: `46ee7f3d3cba1c781c3ce68a8dfdd7251a641247`
- Candidate source: `本地实验/N04-GM-CACHE-STREAMING-X/V002/submission.asc`
- Source SHA256: `dcbc9bfbb710daa6829922dac2a762e84cbc54b32f30d64ac6397c056a2dc86e`
- Change: one `outputGm_.SetL2CacheHint(CACHE_MODE_DISABLE)` call in `ProcessWideLowPrecision`.
- V001 input hint is retained; no arithmetic, tiling, prefetch, DMA descriptor, or parameter-residency change.

## Execution State

- Host: `hwnput3` through `cann-server3`, device d1, Ascend910B3.
- CANN: `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`.
- Remote run directory: `~/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V002/46ee7f3d`.
- Preflight: completed and saved in `preflight-d1.txt`; no N04 Runner was present.
- Compile: NOT_STARTED.
- Correctness: NOT_STARTED.
- Local: NOT_STARTED.
- Device context: d1 HBM 59718/65536 MiB used, AICore 28%; existing process PID 2999855 was left unchanged. Host disk reported about 43 GB free.

## Next Action

Run the existing adapter/device and paired CMake entries in the V002 remote directory, then run Correctness and the interleaved Parent/Candidate Local cases. Save raw output, return codes, post-run device state, and the final result before the next single-factor revision.
