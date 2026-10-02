# SYNC-TOPOLOGY-CHAMPION-X V001 Same-Binary Shape Block

## Source and lease identity

- Build/Correctness `RUN_ID`: `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-6e650cf5-20261002T231909Z`.
- Candidate SHA256: `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`.
- Direct Parent: `R31B V011`.
- Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Device lease: `M1-SYNC-V001-D4-PERF-20261002T232615Z`.
- Lease status: `RELEASED`, recorded by Main schedule commit `18dd2485`.
- Main's handoff did not provide a separate timing results-directory `RUN_ID`; the runner output prefix was `SAME-BINARY-SETENV`.

## Parent same-binary result

- Target: `[2,12288]` FP16, device 4, Parent executable from the Build/Link run above.
- First invocation omitted `source set_env.sh`; the loader reported missing `libruntime.so`, return code `127`. The runner did not start and produced no samples.
- Second invocation sourced `set_env.sh`, used the new prefix `SAME-BINARY-SETENV`, completed 45 warmups and 62 Parent device-event samples, and passed Parent correctness.
- `SAME_BINARY_RESULT=FAIL`.
- `MAD/median=0.062929`; block drift `=0.292906`; Parent block medians were `10.98 us` and `8.42 us`.
- Block drift exceeds `0.25`, so the shape classification is `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`; the Local status is `MEASUREMENT_BLOCKED`.
- Parent window qualification and Parent/Candidate pairs were not run. Candidate timing was not run. No Local score was produced and this is not a Candidate rejection.

## Load and retained evidence

- Device 4 HBM before/after: `59190/65536 MB` used.
- AICore before/after: `0%`.
- `VLLMEngineCor` PID `2999855` remained unchanged; no SYNC process remained after the run.
- Available disk before/after: `616 GB`.
- Main reports raw samples, jitter, runner log, and pre/post snapshots retained in the lease result directory under `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/`.
- The report supplied the prefix `SAME-BINARY-SETENV` but omitted the distinct results-directory `RUN_ID`; the exact directory suffix is therefore not asserted here.
