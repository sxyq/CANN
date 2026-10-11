# N04 V016 Results

## Candidate

- Route: `N04-GM-CACHE-STREAMING-X`.
- Direct parent: N04 V015, source commit `1516f32f5`.
- Candidate source commit: `6bf56393e`.
- Candidate source: `../submission.asc`.
- Candidate source SHA-256: `a2d30d539378dcce1df4abbd2fc534306859c3350fbf216455b078d99e9a4de3`.
- Single change: add `biasGm_.SetL2CacheHint(CACHE_MODE_PERSISTENT)` in the wide low-precision path. Existing gamma, x, residual, and output hints remain unchanged.
- Arithmetic, Vector instructions, Scalar indexing, tile traversal, DMA descriptors, and parameter residency are unchanged.

## Build

- Host: `hwnput3` via `cann-server3`; CANN `8.5.0.alpha002`; SoC `Ascend910B3`.
- Parent input: R31B V011 Champion SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate input: N04 V016 SHA-256 `a2d30d539378dcce1df4abbd2fc534306859c3350fbf216455b078d99e9a4de3`. Remote paths and all paired Runner input identities are recorded in `remote-inputs.txt`.
- Existing `paired_runner_v016` source and CMake entry were reused. Initial configure failed because the archived entry is named `paired-CMakeLists.txt`; the first Compile then exposed missing CANN environment initialization and missing C++ standard header search paths. All failed outputs are preserved separately.
- Compile passed after initializing CANN 8.5 and using the route's established `CPATH`; return code 0. See `configure-attempt1-missing-cmakelists.log`, `configure-retry.log`, `paired-build.log`, `paired-build-env-retry.log`, and `paired-build-env-cpppath.log`.
- Executable: `artifacts/paired_runner_v016`, SHA-256 `6c664ccbc3a07f5d653afc55674f3bc34287851a7bd8dcf8dd6eff89bdfb9632`.

## Correctness

- Status: PASS, Champion and Candidate across all four shapes on d7; Runner return code 0.
- Raw output: `correctness-v011-n04v016.log`.
- A first launch failed in the dynamic loader before device execution because the CANN bundled `libstdc++` was selected without `GLIBCXX_3.4.29`. That output is preserved in `correctness-loader-env-failure.log`. Correctness passed with the system C++ runtime first in `LD_LIBRARY_PATH` and the CANN 8.5 runtime libraries retained.
- Device snapshots are in `pre-correctness-d7-usage.txt`, `pre-correctness-d7-processes.txt`, `post-correctness-d7-usage.txt`, and `post-correctness-d7-processes.txt` with adjacent clock files.

## Paired Local

- Status: NOT RUN. No paired timing was started.
- The closest pre-Local snapshot (`pre-local-d7-clock.txt`, `pre-local-d7-usage.txt`, and `pre-local-d7-processes.txt`) records `2026-10-11T22:03:06+00:00`, AICore 13%, AIVector 17%, HBM bandwidth 0%, and HBM use 37%. The earlier observation at 21:59 recorded AIVector 9%; activity rose again before the run. These measurements show active shared compute despite zero reported HBM bandwidth, so timing was deferred.
- The existing Python process PID 439848 (21,260 MiB) remained untouched; no paired Runner process was present.
- A saved read-only recheck at `local-defer-recheck-clock.txt` (`2026-10-11T22:10:03+00:00`) momentarily showed AICore 0%, AIVector 3%, and HBM bandwidth 0%. The next pre-Local snapshot at `pre-local-d7-final-clock.txt` (`2026-10-11T22:12:27+00:00`) rose to AICore 9%, AIVector 15%, and HBM bandwidth 9%; the Python PID remained present. This renewed shared activity kept Local unstarted. The final device and disk readings are in `pre-local-d7-final-*` and `server-space-pre-local-final.txt`.
- On resume, the fresh snapshot at `pre-local-d7-resume-20261012-clock.txt` (`2026-10-11T22:26:27+00:00`) showed AICore 7%, AIVector 0%, and HBM bandwidth 8%, with the existing Python PID still present and no V016 Runner process. Shared compute remained active, so no timing was started. The full snapshot is preserved in `pre-local-d7-resume-20261012-*`.
- `LOCAL_SCORE=NONE` means no Local performance conclusion is available. Resume with a fresh read-only d7 snapshot when activity is suitable, then use the existing Runner for 45 warmups and 21 alternating pairs per case. Preserve every sample and post-run state.

## Resource State

- Immediately before V016 inputs were added, `/home/data4t2` reported 100% use with 16,609,017,856 bytes available; the existing N04 run tree occupied 38 MB. The build completed with no cleanup or changes to other data.
- After build, `/home/data4t2` still reported 100% use and 16,604,868,608 bytes available. On the 22:10 recheck it reported 16,600,543,232 bytes available; the 22:12 snapshot reported 16,596,586,496 bytes; the 22:26 snapshot reported 16,590,204,928 bytes available. No files outside this V016 run directory were removed or changed.
- d7 remained at 37% HBM use. All local and post-run data generation remains limited to the available server space; the device and Python process were not modified.

## Evidence Files

- Input identities: `remote-inputs.txt`.
- Configure/build and executable: `configure-attempt1-missing-cmakelists.log`, `configure-retry.log`, `paired-build.log`, `paired-build-env-retry.log`, `paired-build-env-cpppath.log`, `build-identity.txt`, `artifacts/paired_runner_v016`.
- Correctness and device state: `correctness-loader-env-failure.log`, `correctness-v011-n04v016.log`, `pre-correctness-*`, `post-correctness-*`.
- Local pre-run state: `pre-local-d7-initial-*`, `pre-local-d7-*`.
- Existing paired entry sources: `paired_runner_v016.asc`, `paired_runner.cpp`, `paired_runner_abi.h`, and `paired-CMakeLists.txt`.
