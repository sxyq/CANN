# Parent Runner Contract Build

- Date: `2026-10-08` UTC
- Worktree / branch: `/home/data4t2/lelinfeng/cann-r-w4-5-aligned-tail` / `route/r-w4-5-aligned-tail-datacopy-x`
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Scope: route-owned build and host-runner contract only; no Parent/kernel source edit, Candidate source, NPU execution, Local, or Online action

## Contract

- CMake builds only `parent_runner` by default. Candidate target generation is opt-in and was disabled for this build.
- `build_exact_parent.sh` rejects any source whose SHA256 differs from the exact Parent hash, uses a fresh build directory, and passes the same expected hash into CMake.
- Compile-time assertions check `TensorInfo` and `TensorGroupInfo` layout and the full `run_kernel` function signature against the definitions used by the included Parent.
- The generated runner embeds the kernel-source SHA256 and prints it at startup.

## Build Result

- Result: `PASS`
- Command: `bash 本地实验/ALIGNED-TAIL-DATACOPY-X/V001/support/build_exact_parent.sh`
- Raw log: `parent-contract-build-20261008.log`
- Build directory: `build-parent-contract-20261008.hRrgdj/`
- Runner SHA256: `92c12270294e4326a2a26c63143af5ff4357a1cacb85a1022203163186924a2b`
- Toolchain: CANN `ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest`; `bisheng` 15.0.5; CMake 3.22.1; `Ascend910B3`
- The fresh build tree contains only the Parent runner target; no `candidate_runner` source or target was generated.

## Gate

The ABI assertions passed, so this check found no layout or function-signature discrepancy between this runner translation unit and the included Parent. The build pins source identity and excludes Candidate target generation, but it does not change runtime launch, memory, or stream behavior. It therefore does not establish a behavioral harness fix and does not justify repeating the unchanged NPU correctness probe.

The latest exact-Parent full-suite result remains `FAIL`: `60/66` pass, with the same six FP32-wide failures at `(4,8193)`, `(2,16383)`, `(2,16384)`, `(2,16385)`, `(1,32768)`, and `(1,32769)`. The preserved repeated-output diagnostic still cannot assign root cause between Parent kernel behavior and runner/runtime behavior.

- Parent Local: `NOT_RUN`
- Candidate V001: `NOT_CREATED`
- Local score: `NONE`
- Online: `NOT_RUN`
- Blocker: no concrete behavioral harness/ABI mismatch has been demonstrated; a runner/runtime contract or comparison proving such a mismatch is needed before another Parent correctness execution.
