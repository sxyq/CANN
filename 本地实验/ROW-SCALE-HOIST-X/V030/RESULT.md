# V030 Result

## Outcome

- Local compile: PASS on `hwnput3` using CANN 8.5.T8.0.B060 (`version_dir=8.5.0.alpha002`) after sourcing `/usr/local/Ascend/ascend-toolkit/set_env.sh`. Both `device` and `submission` targets built. The two earlier SSH/DNS failures are retained and did not start CMake.
- Correctness: PASS for Parent and Candidate on device 0, FP16 `[128, 2048]`; both matched ratio `1.0`, maximum absolute error `0.001953125`.
- Local: `NEEDS_ONE_MORE_LOCAL`. The prescribed P-C-P-C-P-C run completed with 96 raw samples per arm. Descriptive pooled medians were `17.34 us` Parent and `11.97 us` Candidate, equivalent to a `-30.968858132%` candidate-vs-parent latency delta and route-local speedup index `144.862155388`.
- Do not promote V030 to Local Best. Parent block medians drifted `19.85 → 18.16 → 8.559999 us`; paired-block deltas were `-9.471028%`, `-44.933921%`, then `+17.523367%`. The final block reversed direction. Pooled CV was `1.477201` Parent and `1.534666` Candidate.
- Device 0 AICore/HBM-bandwidth utilization changed from `37%/26%` before the window to `0%/0%` afterward; HBM use remained approximately `60.2/65.5 GB`, and other-user NPU/host processes were present. Treat the pooled gain as contention- and time-variant, not causal evidence.
- `CURRENT_LOCAL_BEST` remains V026. Official score is unchanged; Online was forbidden and not run.

## Evidence

- Compile output: `compile-v030-local-20261007T045222Z.log`; structured result: `compile-result-local-20261007T045222Z.json`.
- Correctness build/run: `correctness-build-local-20261007T045717Z.log`, `correctness-run-local-20261007T045843Z.log`, and `correctness-result-local-20261007T045843Z.json`.
- Local runner build and all raw timing/load data: `local-build-20261007T050045Z.log` and `local-paired-20261007T050208Z.log`; structured summary: `local-result.json`.
- Candidate and Parent source SHA256 values are `c75d584e86b1bd58cc54066c59691d143c186352bac413f32a275221e2df3ef3` and `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.

## Receipt

```text
ROUTE_EVENT
ROUTE = ROW-SCALE-HOIST-X
REVISION = V030
LAST_ACTION = RESULT
NEXT_ACTION = COMMIT
CHANGE = Move FP16 full-row row-scale multiplication before output conversion
COMPILE = PASS
CORRECTNESS = PASS
LOCAL_SCORE = 144.862155388 (descriptive pooled index; not stable)
LOCAL_DELTA = -30.968858132% (descriptive pooled delta)
CURRENT_LOCAL_BEST = V026
GIT_COMMIT = PENDING
PUSH = NO
BLOCKER = NONE
```

After commit, continue from V026 as the Local Best. V030 needs another Local measurement under stable load before promotion. Online remains forbidden.
