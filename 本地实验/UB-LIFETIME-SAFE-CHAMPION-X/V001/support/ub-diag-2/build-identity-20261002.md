# UB-DIAG-2 build and executable identity

- Date: 2026-10-02 UTC
- Host: `hwnput3` (local execution; `cann-server3` DNS was unresolved)
- Bootstrap commit: `1adcc7e2aec295a8b56043703625f39147fe477b`
- Toolkit: `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`
- SoC / arch: Ascend 910B3 / `dav-2201`
- Build command: `cmake -S . -B build && cmake --build build -j2`
- Complete build log: `build-20261002.log`
- Result: `BUILD PASS` for both exact-source targets; only non-fatal `cce_global attribute ignored` compiler warnings were emitted.

| target | exact source | source SHA256 | executable | executable SHA256 | bytes |
|---|---|---|---|---:|---:|
| `ub_diag_2_parent` | `../../parent-submission.asc` (`R31B V011`) | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` | `build/ub_diag_2_parent` | `20fd2b283f3ca4cf3d9f07a32e92fadfc78d49428045473ae1a3ae0856811adf` | 485856 |
| `ub_diag_2_candidate` | `../../submission.asc` (`V001`) | `9b73bb5626b5e98be53faeb91eba024b2589bf0b8b5b46e36c48ec18e009f989` | `build/ub_diag_2_candidate` | `9c80858325e904cc463f86e9535f39af72b351f3a04d658697450c37d6e6b06b` | 485560 |

The ignored `build/` directory is retained locally for the correctness run; this
identity record and the complete text log are the committed build evidence.
