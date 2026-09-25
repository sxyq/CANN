# V001 Runner Preparation Record

- Route/revision: WIDE-X-FRESH4 / V001.
- Candidate source SHA256: `f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895`.
- Direct Parent source SHA256: `5d0ee01165e46a281cbb7d1605feba3605ad59883845400a063e97704f27f2be`.
- Both source files were verified against these identities in the Route worktree and in the server3 staging attempt.
- Cleanup path: stream drain is attempted first. A drain error is printed and skips event destruction, device-buffer release, stream destruction, device reset, and runtime finalization. The already selected process exit code is retained; a drain error creates a nonzero result only when the run had no earlier failure.
- Server3 build: not completed. The first attempt stopped before CMake because the installed toolkit did not provide `set_env.sh`; no compiler or linker was invoked. The build script now has a fallback based on the installed toolkit package directory.
- Follow-up server3 build: unavailable after the SSH agent connection closed. The fallback has only passed shell syntax validation; CANN compile/link remain unverified.
- Local build tools: `clang++` and `g++` are present; CMake and local ACL headers are absent, so this host cannot compile or link the production runner.
- Runner execution, NPU correctness through this runner, and performance timing were not run. No current Main device lease was present.
- V001 Candidate and Parent source files were not changed.
- Fresh Blind review note: while locating the problem definition, an archived problem-analysis document was opened and included material beyond the core problem statement. No other Route kernel source or historical champion source was read. Main should decide whether this affects the Route's Fresh Blind classification.

## Server3 paired-runner build retry (2026-09-25)

- Connected through `cann-server3` to `hwnput3`; CANN 8.5.0.alpha002 and its Ascend CMake package were present. The toolkit did not contain `set_env.sh`.
- The first paired-runner build configured successfully but both ASC translation units failed to find `<cstdint>`. The system has GCC 11 headers; `bisheng` selected a GCC 12 installation by default. The complete output is retained in `../logs/server3-build-attempt-01.log`.
- `build_server3.sh` now derives the host GCC version and multiarch paths and exports C++ headers, C headers, and library paths. A compiler syntax probe including `<cstdint>` passed with these paths.
- The rebuild passed the `<cstdint>` stage but failed while resolving ASC kernel metadata for the renamed Parent and Candidate entry points (`wide_x_fresh4_kernel_parent` and `wide_x_fresh4_kernel_candidate`); `__origin__...` symbols remained undefined. No paired-runner executable or shared kernel libraries were linked. Full output is retained in `../logs/server3-build-attempt-02.log`.
- No paired-runner binary was executed and no NPU runtime, correctness run, or timing sample was started. The Parent and Candidate source files remain unchanged at their declared SHA256 values.

## Standard ASC registration and completed link

- CANN 8.5's installed `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/tikcpp/ascendc_kernel_cmake/ASC_CMake/FindASC.cmake` defines `ascendc_library()`: it marks the `.asc` source as language ASC, creates the library target, and attaches the CANN runtime/registration link interface. CMake's ASC language rule invokes `bisheng -x asc`, where ASCPLUGIN derives `kernelInfo` and the origin stub from the kernel declaration and its `<<<>>>` launch.
- The failure came from macro-renaming the `__global__` kernel as well as the host wrapper. Both route sources now reach this build unchanged; the paired support CMake gives only `run_kernel` a side-specific name and leaves `wide_x_fresh4_kernel` intact. No origin symbol was written or synthesized by hand.
- Attempt 03 tried the CANN compile-options helper for `--npu-arch`; this toolkit rejects that option inside the helper's `-Xaicore-start/end` group. The CMake file retains the supported direct ASC language option for architecture selection. Attempt 03 output is retained in `../logs/server3-build-attempt-03.log`.
- Attempt 04 successfully compiled and linked both registered ASC libraries, but the ACL runner inherited the libraries' full CANN device-runtime interface and the final host link then required driver symbols. Attempt 04 output is retained in `../logs/server3-build-attempt-04.log`.
- Attempt 05 links the runner to the two produced library files and the ACL host library, while keeping the full CANN interface on each registered ASC library. It completed ASC compilation, both shared-library links, and runner link. The runner and both libraries have no unresolved `ldd` dependencies. Output and artifact hashes are retained in `../logs/server3-build-attempt-05.log`.
- Server-side source identities at build time: Parent `5d0ee01165e46a281cbb7d1605feba3605ad59883845400a063e97704f27f2be`; Candidate `f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895`. The runner SHA256 is `a74faf2fcf05819db15b5d0f96401f4aa1d9e09b6a54d88be870918a953eb3c4`; Parent library SHA256 is `9f03f15232d984577e559a64e90ec4e550fbef9392d6edb7eedd47ddc530071e`; Candidate library SHA256 is `c30be731387e0672359f47e748d65f67b7ce0e3f294dbc4f0fe986934b0890ab`.
- The paired runner was not executed. No ACL runtime initialization, NPU correctness operation, or timing sample was performed. V001 Candidate and BUILD-FIX-001 Parent sources remain unchanged.
