# R31A V021 correctness-only runner handoff

Date: 2026-09-26

## Scope and outcome

V021 kernel source remains unchanged. A runner-only `correctness DEVICE WIDTH` path was added to `paired_probe_main.inc`. It skips warmup, event creation, sample collection, and timing output; it dispatches V016 and V021 separately with the existing device-output sentinel and full-output comparison.

The regular server3 CMake target returned exit code 2 after the CANN `bisheng` Clang frontend exited 139 while compiling `paired_probe_v021.asc`. That failed attempt is retained. Building only the generated C++ object and link rules then succeeded with exit code 0, using the already-built, identity-verified Parent and Candidate modules. This produced an ELF matching the current runner source.

## Source and executable identity

| Component | Exact source | SHA-256 |
|---|---|---|
| Parent V016 kernel | `phase4/local/R31A/V021/support/parent_v016_submission.asc` | `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0` |
| Candidate V021 kernel | `phase4/local/R31A/V021/submission.asc` | `4f5bfc319b72d1f0bcfd453bc92e80ac64216719898757292daf1aa1b73b6063` |
| Paired host bridge | `phase4/local/R31A/V021/support/paired_probe.cpp` | `da39bdde59ba89a1bd6925542dabe2b546e3a4553384ea190a528f9fa9d57d9f` |
| Correctness/timing main include | `phase4/local/R31A/V021/support/paired_probe_main.inc` | `28c5dd26d3226c8f36668c3e56d9cf0126574e2785868fb18d02e134bf81dcd8` |
| Parent registration wrapper | `phase4/local/R31A/V021/support/paired_probe_v016.asc` | `dd45d48c000b77f9fd47f8c9e050a39ad320da13319553f0722aad78e05baecc` |
| Candidate registration wrapper | `phase4/local/R31A/V021/support/paired_probe_v021.asc` | `4ccb96b2a9f5abf6b5f116a1bf2ff3fdff982ae42dbdac9f2f5c39a2e4541df1` |
| Existing Parent module ELF | server3 `probe-build/libr31a_paired_v016.so` | `11d7a2cb60912ce23b05a768c9076abcf2c513ad8b0844cc062d158082a4d5ea` |
| Existing Candidate module ELF | server3 `probe-build/libr31a_paired_v021.so` | `9bacd8cd72b8db5c6b8a1574ce199c674b18bc2bcf858cfb91ade29f71d24674` |
| Previous runner ELF, before this change | server3 `probe-build/r31a_paired_probe` | `d11fa9aa541e5c75dc38bd509cd515ca44347162741f6b93b7a0b371aecfde11` |
| Current runner ELF | server3 `probe-build/r31a_paired_probe` | `7dbde489de951be883fe38d6cbc991800c0451a97931af8ac036c339f0e61f9a` |

The local and server3 SHA-256 for the changed main include match. The current runner ELF was compiled from `paired_probe.cpp` including this main include and linked against the Parent and Candidate module ELFs listed above.

## Verification

- Regular targeted server3 command: `cmake --build /home/data4t2/lelinfeng/phase4-worktrees/R31A/V021-direct/probe-build --target r31a_paired_probe --verbose`; command exit code 2. The compiler diagnostic records Clang frontend exit code 139 on the V021 paired ASC translation unit.
- Isolated host runner build used the generated `CMakeFiles/r31a_paired_probe.dir/build.make` C++ target only, compiling `paired_probe.cpp` and linking with the existing module outputs; exit code 0. The ELF SHA-256 is `7dbde489de951be883fe38d6cbc991800c0451a97931af8ac036c339f0e61f9a`.
- GCC 11 host bridge suite rerun: 4/4 PASS, process exit code 0. It covers missing library, missing symbol, V016 dispatch, and V021 dispatch; it does not execute ACL or NPU code.
- `npu-smi info` snapshots at 2026-09-26T10:12:39Z and 10:14:11Z, immediately before the runs, showed d7 AICore 0%, HBM 3430-3431/65536 MB, and `No running processes found in NPU 7`. Post-run snapshots at 10:17:09Z and 10:20:36Z showed the same process-free state.
- Correctness-only commands used the current ELF from its build directory with CANN `LD_LIBRARY_PATH` set: `env --chdir=<probe-build> LD_LIBRARY_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64 ./r31a_paired_probe correctness 7 32768` and the same command with width `24576`. D=32768: V016 PASS and V021 PASS, both `max_abs=4.83928943e-07`, process exit code 0. D=24576: V016 PASS and V021 PASS, both `max_abs=3.48268474e-07`, process exit code 0. Both runs report `timing_samples=0`.
- Two earlier launch attempts are retained in the correctness log: the first lacked `LD_LIBRARY_PATH` and could not load `libascendcl.so`; the second placed `env` options in the wrong order and did not find the executable. Both failed before ACL initialization. No same-binary qualification or timing ran.
- `phase4/research/R31A/next-hypotheses.md` is unchanged; it already contains three screened, distinct Route-local hypotheses. No new evidence was found to add another.

## Route-local files changed this turn

- Modified: `phase4/local/R31A/V021/support/paired_probe_main.inc`.
- Added build log: `phase4/local/R31A/V021/support/build-correctness-only-cann-server3-20260926.log`.
- Added isolated C++ build/link log: `phase4/local/R31A/V021/support/build-cxx-runner-isolated-cann-server3-20260926.log`.
- Added identity log: `phase4/local/R31A/V021/support/build-identity-correctness-only-cann-server3-20260926.log`.
- Added host test log: `phase4/local/R31A/V021/support/host-tests-correctness-only-cann-server3-20260926.log`.
- Added d7 snapshots: `phase4/local/R31A/V021/support/npu-smi-d7-pre-correctness-only-cann-server3-20260926.log` and `phase4/local/R31A/V021/support/npu-smi-d7-post-correctness-only-cann-server3-20260926.log`.
- Added runner results: `phase4/local/R31A/V021/support/correctness-only-d7-cann-server3-20260926.log`.
- Added this handoff: `phase4/local/R31A/V021/support/correctness-only-handoff-cann-server3-20260926.md`.
- `identity-correctness-only-cann-server3-20260926.log` retains the initial incomplete checksum command; use the build identity log and this handoff for the complete identities.

No Candidate kernel, shared control file, research file, or performance revision was changed. The files listed here are support-runner code and Route-local evidence only.
