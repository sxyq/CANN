# N04 V004 Handoff

V004 uses V003 as Parent and changes only the existing output-path cache mode from `CACHE_MODE_LAST` to `CACHE_MODE_NORMAL`. The input disable Hint remains unchanged. Candidate source commit is `409df41bee7306ee1ddea8de59e9d0008138b43c`, source SHA256 is `efaae996d2851b408bb690b13103410a32f65dfe029973ebea7a688eda8f4f35`, and the paired harness mapping is `d74d9d3eff21646b6137ad64c2238fca2aa534e1`.

Compile and paired build passed. Correctness passed 4/4. Local completed 21 alternating pairs for each public case after 45 warmups. Parent/Candidate medians, deltas, MAD, CV, and the raw evidence are in `RESULTS.md` and `local-paired.log`; `LOCAL_SCORE=NONE` because of sustained d1 load and large outliers.

The remote run directory is `~/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V004/409df41b`. No N04 process remains. The next revision may change this existing output Hint mode only.
