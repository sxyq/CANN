# SYNC-TOPOLOGY-CHAMPION-X V001 Same-Binary Shape Block

## Source and lease identity

- Build/Correctness `RUN_ID`: `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-6e650cf5-20261002T231909Z`.
- Candidate SHA256: `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`.
- Direct Parent: `R31B V011`.
- Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Device lease: `M1-SYNC-V001-D4-PERF-20261002T232615Z`.
- Lease status: `RELEASED`, recorded by Main schedule commit `18dd2485`.
- No dedicated same-binary `RUN_ID` was allocated. The evidence is stored in the Build/Correctness results directory `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-6e650cf5-20261002T231909Z`, under `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/`.

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
- The raw samples, jitter summary, runner output/status, and snapshots are retained in the result directory above. The raw/jitter files are also copied into this Revision directory and their local SHA256 values match the server3 originals.
- Exact result files: `SAME-BINARY-SETENV-raw.tsv`, `SAME-BINARY-SETENV-jitter.txt`, `same-binary-runner-setenv.log`, `same-binary-status-setenv.txt`, `pre-same-binary-setenv-device.txt`, `pre-same-binary-setenv-disk.txt`, `pre-same-binary-setenv-project-usage.txt`, `post-same-binary-setenv-device.txt`, `post-same-binary-setenv-disk.txt`, `post-same-binary-setenv-processes.txt`, and `post-same-binary-setenv-project-usage.txt`.
