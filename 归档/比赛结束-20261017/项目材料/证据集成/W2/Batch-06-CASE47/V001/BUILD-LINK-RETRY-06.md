# CASE47 V001 Build/Link Retry-06

DATE: 2026-10-03
STATUS: CANDIDATE_AND_PARENT_BUILD_LINK_PASS
SOURCE_COMMIT: `5a2906d64f69b17d00e358ab93e4b71182b68421`
Candidate SHA256: `be313f80e5b088f1fe907f2f4221c61ab59ef720ce68cc9f9c941be20239c2db`
Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
Support CMake SHA256: `aaa38d3ef0ca62f402b0d4b8bacf4f56f6563f43086d69dbe43ed0ead72307ba`

## Environment

- Host: `hwnput3`, `aarch64`; SoC: `Ascend910B3`; build device snapshot: d3.
- CANN/Toolkit: `8.5.0.alpha002`, `/usr/local/Ascend/ascend-toolkit/latest`.
- At compile preflight, d3 used `60219/65536 MB` HBM, leaving `5317 MB`; AICore was 22%. Project disk had `582 GB` available. The post-build HBM reading remained `60219/65536 MB`.
- No CMake, gmake, bisheng, or linker process was present in the compile preflight or post-build process snapshots.

## CMake Change

`case47_timing_runner` keeps `-Wl,-rpath-link,/usr/local/Ascend/driver/lib64/driver`. Its `BUILD_RPATH` now also includes the confirmed driver and common library directories, matching the successful Sync-Topology support runner. The retry-04 link line lacked both driver paths and failed to locate `libascend_hal.so`, leaving `drvHdc*` and `hal*` references unresolved.

The confirmed library is `/usr/local/Ascend/driver/lib64/driver/libascend_hal.so`, an AArch64 shared object. Its `ldd` dependencies resolve. No system configuration or global environment was changed.

## Configure And Build

The first retry-06 Candidate configure used the sourced CANN environment without the ASC package search arguments and returned RC=1 (`ASCConfig.cmake` not found). Its log is retained. The next configure used the package paths confirmed in retry-04's successful CMake cache:

```text
-DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/latest
-DCMAKE_PREFIX_PATH=/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux/tikcpp/ascendc_kernel_cmake
-DASC_DIR=/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux/tikcpp/ascendc_kernel_cmake
```

- Candidate configure: RC=0.
- Parent configure: RC=0.
- Candidate targets `case47_candidate_kernel`, `case47_v001_correctness`, `case47_timing_runner`: RC=0.
- Parent target `case47_parent_kernel`: RC=0.
- Generated timing link line contains the driver `-rpath-link` and RUNPATH entries for toolkit `lib64`, toolkit `aarch64-linux/lib64`, driver `lib64/driver`, and driver `lib64/common`.
- Under the sourced CANN environment, `ldd` resolved all dependencies for both host executables, including `libascend_hal.so` at the confirmed driver path. No executable was launched.

## Build Identities

| Output | SHA256 | ELF identity |
|---|---|---|
| `libcase47_candidate_kernel.so` | `fb7d52da5ad88976a4daa7ce89a33759ea1e78c6e5aeb2ef523c9098d3292c06` | AArch64 shared object |
| `libcase47_parent_kernel.so` | `3de95511fd6437956a8bd48fb2ece9a1f5d0f226f568ef18d129d57db1e77c24` | AArch64 shared object |
| `case47_v001_correctness` | `9e1f8f0b67579651e3267385b9a0e3ed05a9d1ef28796b2f15cbd9a73b157ff7` | AArch64 executable; Build ID `c6548149e6cba0d5d3e08a6f76c96df0343a5cc2` |
| `case47_timing_runner` | `d76923bcdcd791134645e34155cfb646730f822baf73b7ec5bfb5b80d68eda0f` | AArch64 executable; Build ID `1550bfa4235c5bc8e731bfc799976991d575b016` |

## Remote Evidence

Root: `/home/data4t2/lelinfeng/cann/server_runs/CASE47-SMALL-CLUSTER-CHAMPION-X/V001/timing-build-20261003T134147Z`

- Configure logs: `logs/cmake-configure-retry-06-candidate.log` (RC=1), `logs/cmake-configure-retry-06-candidate-attempt-02.log` (RC=0), `logs/cmake-configure-retry-06-parent-attempt-02.log` (RC=0).
- Build logs: `logs/build-retry-06-candidate-attempt-02.log` (RC=0), `logs/build-retry-06-parent-attempt-02.log` (RC=0).
- Identity/dependencies: `logs/executable-identity-retry-06.log`, `logs/runtime-dependencies-retry-06.log` (RC=0 under sourced CANN environment).
- Resource and process snapshots: `logs/npu-smi-retry-06-compile-pre.txt`, `logs/npu-smi-retry-06-parent-pre.txt`, `logs/npu-smi-retry-06-post.txt`, `logs/disk-retry-06-compile-pre.txt`, `logs/project-usage-retry-06-pre.txt`, `logs/project-usage-retry-06-post.txt`, `logs/build-processes-retry-06-compile-pre.txt`, `logs/build-processes-retry-06-post.txt`.
- retry-01 through retry-05 artifacts and the retry-06 first configure log were retained.

## Scope

This retry completed Build/Link only. No NPU correctness, timing, core query, or executable run was performed.
