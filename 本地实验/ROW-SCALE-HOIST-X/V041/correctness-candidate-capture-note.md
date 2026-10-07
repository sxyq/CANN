# V041 Candidate Capture Attempt

The first Candidate invocation for FP32 `[128,16384]` selected `ProcessWideFp32FullCacheRows`, reported `candidate_source_delta_executed=true`, and returned:

```text
CORRECTNESS_RESULT={"status":"FAIL","device":0,"dtype":"fp32","shape":[128,16384],"vector_cores":40,"matched_ratio":0.189493656,"atol":1.52587891e-05,"rtol":0.0009765625,"max_abs_error":4.00334167,"max_abs_error_limit":0.00999999978}
```

The initial `tee` destination had a path typo, so this output was not appended to the primary log on that invocation. The Candidate was rerun with the corrected destination; that independent result is retained in `correctness-local-20261007T232414Z.log`.
