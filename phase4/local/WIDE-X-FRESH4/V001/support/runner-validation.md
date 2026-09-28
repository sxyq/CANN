# V001 Runner Preparation Record

Current status: see the 2026-09-26 revalidation below. Earlier sections preserve the original attempts.

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
- Attempt 06 reran the committed build script. Compile and link completed with the same source and artifact identities; `ldd` resolved `libascend_hal.so` from `/usr/local/Ascend/driver/lib64/driver/` and found no unresolved dependency on the runner or either library. Full output is retained in `../logs/server3-build-attempt-06.log`.
- The paired runner was not executed. No ACL runtime initialization, NPU correctness operation, or timing sample was performed. V001 Candidate and BUILD-FIX-001 Parent sources remain unchanged.


## Server3 revalidation (2026-09-26)

Current stage: CORRECTNESS_ONLY_PASS_WAITING_FOR_PERFORMANCE_WINDOW. Measurement-only commit `a6a38761c30c985bf6647cc2f9c9e6232b3e0747` added an isolated correctness mode and adopted the current 45-launch warmup once per selected side. Candidate and Parent kernel sources remain unchanged. The existing support/build directory was reused; no second build tree or evidence directory was created.

All three compile phases and all three link phases returned numeric RC 0. The Parent/Candidate module SHAs stayed identical to the previously retained modules. The rebuilt runner SHA is `6328cd48d92036aeb4266248ede569f08484580e0507e185520911ed1a7e6a8b`. All runtime dependencies resolve. Help and two invalid-argument paths passed host-only validation. The exact-source correctness-only run later passed 18/18 cases on d7; same-binary and timing remain unrun.

The existing server build script differs from the local script only in retained validation behavior. It was read and left untouched. The command below explicitly invoked the original CMake object and link rules to capture separate return codes. No installation or cleanup command was run.

### Exact executed build command

```bash
ssh -o BatchMode=yes -o ConnectTimeout=8 cann-server3 'bash -s' <<'WIDE_BUILD'
set -eu
WIDE_SUPPORT=/tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/support
WIDE_BUILD_DIR=$WIDE_SUPPORT/build
WIDE_TOOLKIT=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
test -d "$WIDE_BUILD_DIR"
cd "$WIDE_BUILD_DIR"
wide_stage() {
    local wide_name=$1
    shift
    printf 'STAGE=%s COMMAND=' "$wide_name"
    printf '%q ' "$@"
    printf '\n'
    local wide_rc
    if "$@"; then wide_rc=0; else wide_rc=$?; fi
    printf '%s_RC=%d\n' "$wide_name" "$wide_rc"
    test "$wide_rc" -eq 0 || exit "$wide_rc"
}
printf 'ROUTE=WIDE-X-FRESH4 REVISION=V001 PARENT=BUILD-FIX-001\n'
printf 'MEASUREMENT_COMMIT=a6a38761c30c985bf6647cc2f9c9e6232b3e0747\n'
date -u '+BUILD_STARTED=%Y-%m-%dT%H:%M:%SZ'
hostname
uname -m
sha256sum "$WIDE_SUPPORT/../../BUILD-FIX-001/submission.asc" "$WIDE_SUPPORT/../submission.asc" "$WIDE_SUPPORT/runner_main.cpp" "$WIDE_SUPPORT/CMakeLists.txt" "$WIDE_SUPPORT/build_server3.sh"
test "$(sha256sum "$WIDE_SUPPORT/../../BUILD-FIX-001/submission.asc" | cut -d ' ' -f1)" = 5d0ee01165e46a281cbb7d1605feba3605ad59883845400a063e97704f27f2be
test "$(sha256sum "$WIDE_SUPPORT/../submission.asc" | cut -d ' ' -f1)" = f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895
test "$(sha256sum "$WIDE_SUPPORT/runner_main.cpp" | cut -d ' ' -f1)" = 2c2904615ee6445f5b49c7d862daf453ee8837518ebf6e0732d854e97625b84e
export ASCEND_HOME_PATH=$WIDE_TOOLKIT
export ASCEND_CANN_PACKAGE_PATH=$WIDE_TOOLKIT
export CMAKE_PREFIX_PATH=$WIDE_TOOLKIT/aarch64-linux/tikcpp/ascendc_kernel_cmake
export PATH=$WIDE_TOOLKIT/compiler/ccec_compiler/bin:$WIDE_TOOLKIT/tools/ccec_compiler/bin:$PATH
export CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward
export C_INCLUDE_PATH=/usr/include/aarch64-linux-gnu
export LIBRARY_PATH=/usr/lib/gcc/aarch64-linux-gnu/11:/usr/lib/aarch64-linux-gnu:/lib/aarch64-linux-gnu
export LD_LIBRARY_PATH=$WIDE_TOOLKIT/aarch64-linux/lib64:$WIDE_TOOLKIT/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common
printf 'CANN_ROOT=%s\nSOC=Ascend910B3\nNPU_ARCH=dav-2201\n' "$WIDE_TOOLKIT"
sed -n '1,3p' "$WIDE_TOOLKIT/compiler/version.info"
g++ --version | head -n 1
bisheng --version | head -n 3
wide_stage CONFIGURE cmake -S "$WIDE_SUPPORT" -B "$WIDE_BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DASCEND_HOME_PATH="$WIDE_TOOLKIT" -DASCEND_CANN_PACKAGE_PATH="$WIDE_TOOLKIT" -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH"
wide_stage PARENT_COMPILE make -B -f CMakeFiles/wide_x_fresh4_parent.dir/build.make CMakeFiles/wide_x_fresh4_parent.dir/tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/BUILD-FIX-001/submission.asc.o
wide_stage PARENT_LINK cmake -E cmake_link_script CMakeFiles/wide_x_fresh4_parent.dir/link.txt --verbose=1
wide_stage CANDIDATE_COMPILE make -B -f CMakeFiles/wide_x_fresh4_candidate.dir/build.make CMakeFiles/wide_x_fresh4_candidate.dir/tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/submission.asc.o
wide_stage CANDIDATE_LINK cmake -E cmake_link_script CMakeFiles/wide_x_fresh4_candidate.dir/link.txt --verbose=1
wide_stage RUNNER_COMPILE make -B -f CMakeFiles/wide_x_fresh4_unified_runner.dir/build.make CMakeFiles/wide_x_fresh4_unified_runner.dir/runner_main.cpp.o
wide_stage RUNNER_LINK cmake -E cmake_link_script CMakeFiles/wide_x_fresh4_unified_runner.dir/link.txt --verbose=1
for wide_artifact in libwide_x_fresh4_parent.so libwide_x_fresh4_candidate.so wide_x_fresh4_unified_runner; do
    file "$wide_artifact"
    sha256sum "$wide_artifact"
    readelf -h "$wide_artifact" | awk '/Class:|Type:|Machine:/{print}'
    wide_dependencies=$(ldd "$wide_artifact")
    printf '%s\n' "$wide_dependencies"
    if printf '%s\n' "$wide_dependencies" | grep -q 'not found'; then exit 1; fi
done
sha256sum CMakeFiles/wide_x_fresh4_parent.dir/tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/BUILD-FIX-001/submission.asc.o CMakeFiles/wide_x_fresh4_candidate.dir/tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/submission.asc.o CMakeFiles/wide_x_fresh4_unified_runner.dir/runner_main.cpp.o
sha256sum "$WIDE_SUPPORT/../../BUILD-FIX-001/submission.asc" "$WIDE_SUPPORT/../submission.asc"
printf 'NPU_CORRECTNESS=NOT_RUN\nSAME_BINARY=NOT_RUN\nTIMING=NOT_RUN\n'
date -u '+BUILD_FINISHED=%Y-%m-%dT%H:%M:%SZ'
WIDE_BUILD
```

### Raw build output

```text
ROUTE=WIDE-X-FRESH4 REVISION=V001 PARENT=BUILD-FIX-001
MEASUREMENT_COMMIT=a6a38761c30c985bf6647cc2f9c9e6232b3e0747
BUILD_STARTED=2026-09-26T12:28:21Z
hwnput3
aarch64
5d0ee01165e46a281cbb7d1605feba3605ad59883845400a063e97704f27f2be  /tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/support/../../BUILD-FIX-001/submission.asc
f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895  /tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/support/../submission.asc
2c2904615ee6445f5b49c7d862daf453ee8837518ebf6e0732d854e97625b84e  /tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/support/runner_main.cpp
12d545de1e31049d66e7a51ceda186100188ac10c191bfe4a7985dc305c403f2  /tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/support/CMakeLists.txt
0c046a52b9a6296cbdd2a374d47198e004cf8b0a5cfcc0fbd86c979f5436b351  /tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/support/build_server3.sh
CANN_ROOT=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
SOC=Ascend910B3
NPU_ARCH=dav-2201
Version=8.5.T8.0.B060
version_dir=8.5.0.alpha002
timestamp=20251209_133531043
g++ (Ubuntu 11.4.0-1ubuntu1~22.04.3) 11.4.0
2025-11-27T21:34:53+08:00 clang version 15.0.5 (clang-5c68a1cb1231 flang-5c68a1cb1231)
Target: aarch64-unknown-linux-gnu
Thread model: posix
STAGE=CONFIGURE COMMAND=cmake -S /tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/support -B /tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/support/build -DCMAKE_BUILD_TYPE=Release -DASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DCMAKE_PREFIX_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/tikcpp/ascendc_kernel_cmake
-- Configuring done
-- Generating done
-- Build files have been written to: /tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/support/build
CONFIGURE_RC=0
STAGE=PARENT_COMPILE COMMAND=make -B -f CMakeFiles/wide_x_fresh4_parent.dir/build.make CMakeFiles/wide_x_fresh4_parent.dir/tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/BUILD-FIX-001/submission.asc.o
Building ASC object CMakeFiles/wide_x_fresh4_parent.dir/tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/BUILD-FIX-001/submission.asc.o
PARENT_COMPILE_RC=0
STAGE=PARENT_LINK COMMAND=cmake -E cmake_link_script CMakeFiles/wide_x_fresh4_parent.dir/link.txt --verbose=1
/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng -Wl,-Bsymbolic -shared -Wl,-soname,libwide_x_fresh4_parent.so -o libwide_x_fresh4_parent.so CMakeFiles/wide_x_fresh4_parent.dir/tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/BUILD-FIX-001/submission.asc.o   -L/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64  -L/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64
PARENT_LINK_RC=0
STAGE=CANDIDATE_COMPILE COMMAND=make -B -f CMakeFiles/wide_x_fresh4_candidate.dir/build.make CMakeFiles/wide_x_fresh4_candidate.dir/tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/submission.asc.o
Building ASC object CMakeFiles/wide_x_fresh4_candidate.dir/tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/submission.asc.o
CANDIDATE_COMPILE_RC=0
STAGE=CANDIDATE_LINK COMMAND=cmake -E cmake_link_script CMakeFiles/wide_x_fresh4_candidate.dir/link.txt --verbose=1
/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng -Wl,-Bsymbolic -shared -Wl,-soname,libwide_x_fresh4_candidate.so -o libwide_x_fresh4_candidate.so CMakeFiles/wide_x_fresh4_candidate.dir/tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/submission.asc.o   -L/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64  -L/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64
CANDIDATE_LINK_RC=0
STAGE=RUNNER_COMPILE COMMAND=make -B -f CMakeFiles/wide_x_fresh4_unified_runner.dir/build.make CMakeFiles/wide_x_fresh4_unified_runner.dir/runner_main.cpp.o
Building CXX object CMakeFiles/wide_x_fresh4_unified_runner.dir/runner_main.cpp.o
RUNNER_COMPILE_RC=0
STAGE=RUNNER_LINK COMMAND=cmake -E cmake_link_script CMakeFiles/wide_x_fresh4_unified_runner.dir/link.txt --verbose=1
/usr/bin/c++ CMakeFiles/wide_x_fresh4_unified_runner.dir/runner_main.cpp.o -o wide_x_fresh4_unified_runner  -Wl,-rpath,"/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common:\$ORIGIN:/tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/support/build" libwide_x_fresh4_parent.so libwide_x_fresh4_candidate.so /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascendcl.so -ldl -lm
RUNNER_LINK_RC=0
libwide_x_fresh4_parent.so: ELF 64-bit LSB shared object, ARM aarch64, version 1 (SYSV), dynamically linked, not stripped
9f03f15232d984577e559a64e90ec4e550fbef9392d6edb7eedd47ddc530071e  libwide_x_fresh4_parent.so
  Class:                             ELF64
  Type:                              DYN (Shared object file)
  Machine:                           AArch64
	linux-vdso.so.1 (0x0000ffff934bb000)
	libgcc_s.so.1 => /lib/aarch64-linux-gnu/libgcc_s.so.1 (0x0000ffff933b0000)
	libc.so.6 => /lib/aarch64-linux-gnu/libc.so.6 (0x0000ffff93200000)
	/lib/ld-linux-aarch64.so.1 (0x0000ffff93482000)
	libascendcl.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascendcl.so (0x0000ffff92fe0000)
	libruntime.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libruntime.so (0x0000ffff92f10000)
	liberror_manager.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/liberror_manager.so (0x0000ffff92e90000)
	libprofapi.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libprofapi.so (0x0000ffff92e50000)
	libascendalog.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascendalog.so (0x0000ffff92e00000)
	libmmpa.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libmmpa.so (0x0000ffff92dd0000)
	libascend_dump.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascend_dump.so (0x0000ffff92c90000)
	libc_sec.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libc_sec.so (0x0000ffff92c50000)
	libstdc++.so.6 => /lib/aarch64-linux-gnu/libstdc++.so.6 (0x0000ffff92a20000)
	libmsprofiler.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libmsprofiler.so (0x0000ffff92670000)
	libgert.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libgert.so (0x0000ffff91e10000)
	libascendcl_impl.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascendcl_impl.so (0x0000ffff91d50000)
	librt.so.1 => /lib/aarch64-linux-gnu/librt.so.1 (0x0000ffff91d30000)
	libdl.so.2 => /lib/aarch64-linux-gnu/libdl.so.2 (0x0000ffff91d10000)
	libge_executor.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libge_executor.so (0x0000ffff91c00000)
	libgraph.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libgraph.so (0x0000ffff919e0000)
	libascend_watchdog.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascend_watchdog.so (0x0000ffff919b0000)
	libascend_protobuf.so.3.13.0.0 => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascend_protobuf.so.3.13.0.0 (0x0000ffff91480000)
	libpthread.so.0 => /lib/aarch64-linux-gnu/libpthread.so.0 (0x0000ffff91460000)
	libruntime_common.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libruntime_common.so (0x0000ffff91400000)
	libascend_hal.so => /usr/local/Ascend/driver/lib64/driver/libascend_hal.so (0x0000ffff90f90000)
	libunified_dlog.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libunified_dlog.so (0x0000ffff90f40000)
	libm.so.6 => /lib/aarch64-linux-gnu/libm.so.6 (0x0000ffff90ea0000)
	libgraph_base.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libgraph_base.so (0x0000ffff906e0000)
	libhybrid_executor.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libhybrid_executor.so (0x0000ffff90280000)
	libdavinci_executor.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libdavinci_executor.so (0x0000ffff8fda0000)
	libge_common.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libge_common.so (0x0000ffff8fc60000)
	libge_common_base.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libge_common_base.so (0x0000ffff8f6b0000)
	libexe_graph.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libexe_graph.so (0x0000ffff8f650000)
	liblowering.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/liblowering.so (0x0000ffff8f560000)
	libregister.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libregister.so (0x0000ffff8e8b0000)
	libopp_registry.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libopp_registry.so (0x0000ffff8e820000)
	libplatform.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libplatform.so (0x0000ffff8e6c0000)
	libmetadef.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libmetadef.so (0x0000ffff8e580000)
	libascend_trace.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascend_trace.so (0x0000ffff8db40000)
libwide_x_fresh4_candidate.so: ELF 64-bit LSB shared object, ARM aarch64, version 1 (SYSV), dynamically linked, not stripped
c30be731387e0672359f47e748d65f67b7ce0e3f294dbc4f0fe986934b0890ab  libwide_x_fresh4_candidate.so
  Class:                             ELF64
  Type:                              DYN (Shared object file)
  Machine:                           AArch64
	linux-vdso.so.1 (0x0000ffff93d4f000)
	libgcc_s.so.1 => /lib/aarch64-linux-gnu/libgcc_s.so.1 (0x0000ffff93c50000)
	libc.so.6 => /lib/aarch64-linux-gnu/libc.so.6 (0x0000ffff93aa0000)
	/lib/ld-linux-aarch64.so.1 (0x0000ffff93d16000)
	libascendcl.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascendcl.so (0x0000ffff93880000)
	libruntime.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libruntime.so (0x0000ffff937b0000)
	liberror_manager.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/liberror_manager.so (0x0000ffff93730000)
	libprofapi.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libprofapi.so (0x0000ffff936f0000)
	libascendalog.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascendalog.so (0x0000ffff936a0000)
	libmmpa.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libmmpa.so (0x0000ffff93670000)
	libascend_dump.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascend_dump.so (0x0000ffff93530000)
	libc_sec.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libc_sec.so (0x0000ffff934f0000)
	libstdc++.so.6 => /lib/aarch64-linux-gnu/libstdc++.so.6 (0x0000ffff932c0000)
	libmsprofiler.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libmsprofiler.so (0x0000ffff92f10000)
	libgert.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libgert.so (0x0000ffff926b0000)
	libascendcl_impl.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascendcl_impl.so (0x0000ffff925f0000)
	librt.so.1 => /lib/aarch64-linux-gnu/librt.so.1 (0x0000ffff925d0000)
	libdl.so.2 => /lib/aarch64-linux-gnu/libdl.so.2 (0x0000ffff925b0000)
	libge_executor.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libge_executor.so (0x0000ffff924a0000)
	libgraph.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libgraph.so (0x0000ffff92280000)
	libascend_watchdog.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascend_watchdog.so (0x0000ffff92250000)
	libascend_protobuf.so.3.13.0.0 => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascend_protobuf.so.3.13.0.0 (0x0000ffff91d20000)
	libpthread.so.0 => /lib/aarch64-linux-gnu/libpthread.so.0 (0x0000ffff91d00000)
	libruntime_common.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libruntime_common.so (0x0000ffff91ca0000)
	libascend_hal.so => /usr/local/Ascend/driver/lib64/driver/libascend_hal.so (0x0000ffff91830000)
	libunified_dlog.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libunified_dlog.so (0x0000ffff917e0000)
	libm.so.6 => /lib/aarch64-linux-gnu/libm.so.6 (0x0000ffff91740000)
	libgraph_base.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libgraph_base.so (0x0000ffff90f80000)
	libhybrid_executor.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libhybrid_executor.so (0x0000ffff90b20000)
	libdavinci_executor.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libdavinci_executor.so (0x0000ffff90640000)
	libge_common.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libge_common.so (0x0000ffff90500000)
	libge_common_base.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libge_common_base.so (0x0000ffff8ff50000)
	libexe_graph.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libexe_graph.so (0x0000ffff8fef0000)
	liblowering.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/liblowering.so (0x0000ffff8fe00000)
	libregister.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libregister.so (0x0000ffff8f150000)
	libopp_registry.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libopp_registry.so (0x0000ffff8f0c0000)
	libplatform.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libplatform.so (0x0000ffff8ef60000)
	libmetadef.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libmetadef.so (0x0000ffff8ee20000)
	libascend_trace.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascend_trace.so (0x0000ffff8e3e0000)
wide_x_fresh4_unified_runner: ELF 64-bit LSB pie executable, ARM aarch64, version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux-aarch64.so.1, BuildID[sha1]=6be4364fe832742f3e4ac6d97b06f2d730083541, for GNU/Linux 3.7.0, not stripped
6328cd48d92036aeb4266248ede569f08484580e0507e185520911ed1a7e6a8b  wide_x_fresh4_unified_runner
  Class:                             ELF64
  Type:                              DYN (Position-Independent Executable file)
  Machine:                           AArch64
	linux-vdso.so.1 (0x0000ffffa8e69000)
	libwide_x_fresh4_parent.so => /tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/support/build/./libwide_x_fresh4_parent.so (0x0000ffffa8d80000)
	libwide_x_fresh4_candidate.so => /tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/support/build/./libwide_x_fresh4_candidate.so (0x0000ffffa8cf0000)
	libascendcl.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascendcl.so (0x0000ffffa8ad0000)
	libstdc++.so.6 => /lib/aarch64-linux-gnu/libstdc++.so.6 (0x0000ffffa8890000)
	libm.so.6 => /lib/aarch64-linux-gnu/libm.so.6 (0x0000ffffa87f0000)
	libgcc_s.so.1 => /lib/aarch64-linux-gnu/libgcc_s.so.1 (0x0000ffffa87c0000)
	libc.so.6 => /lib/aarch64-linux-gnu/libc.so.6 (0x0000ffffa8610000)
	/lib/ld-linux-aarch64.so.1 (0x0000ffffa8e30000)
	libruntime.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libruntime.so (0x0000ffffa8540000)
	liberror_manager.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/liberror_manager.so (0x0000ffffa84c0000)
	libprofapi.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libprofapi.so (0x0000ffffa8480000)
	libascendalog.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascendalog.so (0x0000ffffa8430000)
	libmmpa.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libmmpa.so (0x0000ffffa8400000)
	libascend_dump.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascend_dump.so (0x0000ffffa82c0000)
	libc_sec.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libc_sec.so (0x0000ffffa8280000)
	libmsprofiler.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libmsprofiler.so (0x0000ffffa7ed0000)
	libgert.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libgert.so (0x0000ffffa7670000)
	libascendcl_impl.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascendcl_impl.so (0x0000ffffa75b0000)
	librt.so.1 => /lib/aarch64-linux-gnu/librt.so.1 (0x0000ffffa7590000)
	libdl.so.2 => /lib/aarch64-linux-gnu/libdl.so.2 (0x0000ffffa7570000)
	libge_executor.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libge_executor.so (0x0000ffffa7460000)
	libgraph.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libgraph.so (0x0000ffffa7240000)
	libascend_watchdog.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascend_watchdog.so (0x0000ffffa7210000)
	libascend_protobuf.so.3.13.0.0 => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascend_protobuf.so.3.13.0.0 (0x0000ffffa6ce0000)
	libpthread.so.0 => /lib/aarch64-linux-gnu/libpthread.so.0 (0x0000ffffa6cc0000)
	libruntime_common.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libruntime_common.so (0x0000ffffa6c60000)
	libascend_hal.so => /usr/local/Ascend/driver/lib64/driver/libascend_hal.so (0x0000ffffa67f0000)
	libunified_dlog.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libunified_dlog.so (0x0000ffffa67a0000)
	libgraph_base.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libgraph_base.so (0x0000ffffa5fe0000)
	libhybrid_executor.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libhybrid_executor.so (0x0000ffffa5b80000)
	libdavinci_executor.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libdavinci_executor.so (0x0000ffffa56a0000)
	libge_common.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libge_common.so (0x0000ffffa5560000)
	libge_common_base.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libge_common_base.so (0x0000ffffa4fb0000)
	libexe_graph.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libexe_graph.so (0x0000ffffa4f50000)
	liblowering.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/liblowering.so (0x0000ffffa4e60000)
	libregister.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libregister.so (0x0000ffffa41b0000)
	libopp_registry.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libopp_registry.so (0x0000ffffa4120000)
	libplatform.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libplatform.so (0x0000ffffa3fc0000)
	libmetadef.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libmetadef.so (0x0000ffffa3e80000)
	libascend_trace.so => /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libascend_trace.so (0x0000ffffa3440000)
3d9a7b9c6f36bfa6d7c34fcbaac2668546bae8d493a192307864002b554bf362  CMakeFiles/wide_x_fresh4_parent.dir/tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/BUILD-FIX-001/submission.asc.o
0ea11c0b5b8e5d5f1078e47a8fc50d561da8033e493f42dda573285337d7f7de  CMakeFiles/wide_x_fresh4_candidate.dir/tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/submission.asc.o
05fe5921ecc90561ef38acef7b76393621eaab6e7f9dd6e50e5b2121fcc35dd9  CMakeFiles/wide_x_fresh4_unified_runner.dir/runner_main.cpp.o
5d0ee01165e46a281cbb7d1605feba3605ad59883845400a063e97704f27f2be  /tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/support/../../BUILD-FIX-001/submission.asc
f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895  /tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/support/../submission.asc
NPU_CORRECTNESS=NOT_RUN
SAME_BINARY=NOT_RUN
TIMING=NOT_RUN
BUILD_FINISHED=2026-09-26T12:28:53Z

```

### Host-only argument validation and canonical retention

```text
HOST_VALIDATION_STARTED=2026-09-26T12:31:09Z
Usage: ./wide_x_fresh4_unified_runner DEVICE WIDTH DTYPE MODE SIDE WARMUPS SAMPLES BLOCKS OUTPUT.tsv
DTYPE: fp32 | fp16 | bf16; MODE: correctness-only | same | paired; SIDE: parent | candidate | -
correctness-only requires SIDE parent/candidate and WARMUPS=0 SAMPLES=0 BLOCKS=1; no events or timing.
same requires >=45 warmups, >=21 samples, >=2 blocks; paired requires >=4 blocks.
HELP_RC=0
unsupported dtype, mode, side, width, or block count
CORRECTNESS_NONZERO_WARMUP_REJECTION_RC=2
invalid numeric argument or below protocol minimum
TIMING_WARMUP_BELOW_45_REJECTION_RC=2
6328cd48d92036aeb4266248ede569f08484580e0507e185520911ed1a7e6a8b  wide_x_fresh4_unified_runner
9f03f15232d984577e559a64e90ec4e550fbef9392d6edb7eedd47ddc530071e  libwide_x_fresh4_parent.so
c30be731387e0672359f47e748d65f67b7ce0e3f294dbc4f0fe986934b0890ab  libwide_x_fresh4_candidate.so
# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# compile ASC with /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng
ASC_DEFINES = -DFRESH4_LOCAL_BUILD -Drun_kernel=run_kernel_parent -Dwide_x_fresh4_parent_EXPORTS

ASC_INCLUDES = -I/tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/support/../../BUILD-FIX-001/parent-control -I/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/include

ASC_FLAGS = --npu-arch=dav-2201

# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# compile ASC with /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng
ASC_DEFINES = -DFRESH4_LOCAL_BUILD -Drun_kernel=run_kernel_candidate -Dwide_x_fresh4_candidate_EXPORTS

ASC_INCLUDES = -I/tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/support/../../BUILD-FIX-001/parent-control -I/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/include

ASC_FLAGS = --npu-arch=dav-2201

HOST_TEST_NPU_LAUNCHES=0
HOST_VALIDATION_FINISHED=2026-09-26T12:31:11Z
commit fa0794f991d3d23279f7e765ae5175a75834d002

A	phase4/workspaces/WIDE-X-FRESH4/CMakeLists.txt
A	phase4/workspaces/WIDE-X-FRESH4/build_server3.sh
A	phase4/workspaces/WIDE-X-FRESH4/host_probe_shim.h
A	phase4/workspaces/WIDE-X-FRESH4/host_probes.cpp
A	phase4/workspaces/WIDE-X-FRESH4/local_abi_shim.h
A	phase4/workspaces/WIDE-X-FRESH4/wide_x_fresh4.asc
bc4608dcb1028b61de50fc8a74d4a72661945c02c3caccb8b60dd543bdf6677c  -

```

Main retained the six original canonical workspace files at `fa0794f991d3d23279f7e765ae5175a75834d002`. Their original paths and content equivalents remain enumerated in `../local-result.json`; they were not copied here.

The legacy BUILD-FIX-001 executable was found at `/home/data4t2/lelinfeng/phase4-workspaces/WIDE-X-FRESH4/build-server3/npu_correctness_BUILD-FIX-001`, with current SHA `dbd7d619a8e928963df94e0479d242f465de67174291c01b60a514b5c15a4a96`. Its adjacent library still has the recorded `b54a6fbdffcef7a6131ed264c4e45a6ac29467651bd2251fdfc568d47fae1cbe` SHA, but the remote workspace source is now V001 and the current correctness source differs from the retained source. Historical executable/source binding therefore remains MISSING; the new exact-source Parent module above is the prepared control.

## Correctness-only runner qualification (2026-09-26)

The exact-source Parent/Candidate modules and the unified runner were executed on `cann-server3` device 7 under the temporary correctness-only lease `M1-WIDE-X-FRESH4-CORRECT-20260926`. Device 7 was used for correctness only; it remains excluded from performance timing.

| side | dtype × widths | cases | RC=0 | mismatches | nonfinite | result |
|---|---|---:|---:|---:|---:|---|
| BUILD-FIX-001 Parent | FP32/FP16/BF16 × 2048/16384/32768 | 9 | 9 | 0 | 0 | PASS |
| V001 Candidate | FP32/FP16/BF16 × 2048/16384/32768 | 9 | 9 | 0 | 0 | PASS |

Exact runner: `/tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/support/build/wide_x_fresh4_unified_runner`, SHA256 `6328cd48d92036aeb4266248ede569f08484580e0507e185520911ed1a7e6a8b`.

Exact source identities remained unchanged: Parent `5d0ee01165e46a281cbb7d1605feba3605ad59883845400a063e97704f27f2be`; Candidate `f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895`. Runtime libraries used for the successful invocation were the server3 toolkit and driver paths recorded in `source-meta.json`.

Evidence is retained under `../logs/correctness-only-d7-20260926T134500Z/`, with one TSV per side/dtype/width and `runner-console.log`. The first invocation without `LD_LIBRARY_PATH` returned RC=127 before NPU launch; its log is `../logs/correctness-only-loader-failure-20260926.log`. That failure is an environment invocation failure, not a kernel correctness result.

This run qualifies correctness only. Same-binary qualification, raw device-event timing, interleaved Parent/Candidate samples, shape noise floors, Local Best, Online Worthy, and Online submission remain unestablished.
