# V001 Compile

COMPILE=PASS
CONFIGURE_RC=0
BUILD_RC=0
REMOTE_HOST=hwnput3
REMOTE_DIR=/tmp/n05-scalar-address-generation-v001.GK5bB6
CANDIDATE_SHA256=8183a42535b3130263da3dce4999747f555de053194c32a0087f3c1f53b54558
CANDIDATE_REMOTE_ALIAS=R31B-V016-WIDE-TILE-SEED_kernel.asc
CANDIDATE_REMOTE_ALIAS_SHA256=8183a42535b3130263da3dce4999747f555de053194c32a0087f3c1f53b54558

## Attempt 1

The initial CMake call used a temporary directory containing only `submission.asc`. It returned RC=1 because that directory had no `CMakeLists.txt`. Original command output is in `compile-attempt-01.log`.

## Attempt 2

The existing worktree inputs under `本地实验/R31B/V016/support/` were used without retaining copies under N05. Their SHA256 values matched the files uploaded for this build. The existing `CMakeLists.txt`, `device.cpp`, `adapter.cpp`, `compile_adapter_v016.asc`, `link_one.sh`, and `runtime_probe.cpp` were used as input. Candidate was connected under the include name expected by the existing adapter files.

Environment command: `source /usr/local/Ascend/ascend-toolkit/set_env.sh`, with shell nounset disabled. The source returned RC=0 and set `ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest`; it left `SOC_VERSION` empty. The build command set `SOC_VERSION=Ascend910B3`. CMake selected CANN `8.5.0.alpha002`, CMake `3.22.1`, and Bisheng `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/ccec_compiler/bin/bisheng`.

Commands:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target device submission full_link -j2
```

Only the three named targets ran. The default `all` target and runtime probes were not built.

NPU_ID=7
SOC=Ascend910B3
HBM_BEFORE=26034/65536 MB used; FREE_HBM=39502 MB
HBM_AFTER=26034/65536 MB used; FREE_HBM=39502 MB
NPU7_AICORE_FULL_INFO=0% before; 11% after
NPU7_AICORE_USAGE_QUERY=0% before; 1% after
NPU7_AIVECTOR_USAGE_QUERY=0% before; 35% after
NPU7_PROCESSES_BEFORE_AND_AFTER=PID 439848 python3 21260 MB; PID 445986 python 1296 MB
NPU7_PROCESS_CHANGE=NONE
FILESYSTEM=/home/data4t2; 105 GB free before; 104 GB free after; 97% used
REMOTE_TEMP_CLEANUP=PASS; V001 temporary directory removed after raw output and artifact details were saved locally

Build outputs:

- `device.alink`: 183144 bytes; SHA256 `2d57db9d4cf3d0757d016e0c4059212ab889b40fbd8e1f772f85b4bc6cb033b2`
- `submission.alink`: 183144 bytes; SHA256 `90627329d20f68b16c7835314f4d5d15f64422c68787ab23090f0d9e4e723951`
- `device.cpp.o`: 334128 bytes; SHA256 `d22a07d10d53d65fa84a5a3b18eb1458f6966a8753781cc22d9e097358c0c66e`
- `adapter.cpp.o`: 334128 bytes; SHA256 `7d1ea8cefb160a27bc9b81cd9fcd1ebf64520e7fa2ee9916a6befa2b134504b6`

Raw output is in `compile-attempt-02.log`; toolchain and artifact details are in `build-artifacts.txt`.
