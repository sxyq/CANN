# A001 Row-Resident Fused Streaming

This workspace contains the A001 `AddRmsNormBias` Ascend C kernel for
Ascend910B3 / DAV_2201. Each Vector Core owns a contiguous range of complete
rows. The D dimension is never split between cores.

## Build

The compile was run on `cann-server3` with CANN `8.5.0.alpha002`:

```sh
source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh
export ASCEND_CANN_PACKAGE_PATH="$ASCEND_HOME_PATH"
export CPLUS_INCLUDE_PATH="$ASCEND_HOME_PATH/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:$ASCEND_HOME_PATH/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:$ASCEND_HOME_PATH/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$ASCEND_HOME_PATH/aarch64-linux/tikcpp/ascendc_kernel_cmake" \
  -DCMAKE_MODULE_PATH="$ASCEND_HOME_PATH/aarch64-linux/tikcpp/ascendc_kernel_cmake/ASC_CMake"
cmake --build build -j2
```

The captured logs are `build/configure.log` and `build/compile.log`.

