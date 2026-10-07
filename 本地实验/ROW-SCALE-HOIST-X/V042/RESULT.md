# V042 Result

- Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`)
- Change: In `ProcessNarrowMidOverlap`, move the existing FP32 row-scale `Muls` before the parameter-ready MTE2_V wait; the scale value, math, DMA, and synchronization identities are unchanged.
- Compile: PASS for device and submission targets.
- Correctness: Parent and Candidate PASS on FP32 `[40,2050]`, dispatching `ProcessNarrowMidOverlap`; Candidate source delta executed. Maximum absolute errors were `1.54972076e-06` Parent and `1.43051147e-06` Candidate.
- Local: `MEASUREMENT_BLOCKED`; descriptive pooled-median index `112.7739480276`, delta `-11.3270380713%` (Candidate appears faster, but is not a reliable gain).
- Pooled medians: Parent `18.0099995 us`, Candidate `15.97 us`, 96 samples per arm. Pooled CV was `0.398999` / `0.455712`.
- Paired block medians (Parent / Candidate, us): `20.10 / 9.66` (`-51.940296%`), `17.790001 / 17.279999` (`-2.866782%`), `16.230001 / 17.49` (`+7.763395%`). Direction was mixed.
- Parent same-binary stability medians shifted from `18.7799995` to `7.85 us` (difference `10.9299995 us`). Host load was high and increased during sampling; activity and processes were observed on other NPUs. NPU 0 was healthy, idle, and had no process before or after.
- The initial Parent stability attempts exited `127` because `libmsprofiler.so` was not in the shell environment; environment-sourced retries succeeded. The unsupported `npu-smi info -t process -i 0` diagnostic exited `215`; its usage output and the subsequent successful snapshot are retained. These failures are not Candidate regressions.
- `CURRENT_LOCAL_BEST=V026`; Official score absent; Online not run.

Raw compile, correctness, runner build, stability attempts/retries, all paired samples, and before/after load snapshots remain in this directory.
