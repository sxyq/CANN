# Swish Example

## Overview

This example uses the Swish/Silu high-level APIs in a deep learning model activation function scenario to implement activation function computation. The two APIs have a close mathematical relationship:

- **Swish (default)**: `y = x / (1 + exp(-beta * x))`, where beta is an adjustable parameter. In this example, beta=1.702 (GELU approximation).
- **Silu**: A special case of Swish when beta=1, `y = x / (1 + exp(-x))`. It is smoother than ReLU and the gradient does not completely vanish.

This example controls the two modes through the compilation macro `USE_SILU_MODE`, uses float data type, and the input tensor has 32 elements.

## Supported Products and CANN Software Versions

| Product | CANN Software Version |
|------|-------------|
| Ascend 950PR/Ascend 950DT | >= CANN 9.1.0 |
| Atlas A3 Training Series Products/Atlas A3 Inference Series Products | >= CANN 9.0.0 |
| Atlas A2 Training Series Products/Atlas A2 Inference Series Products | >= CANN 9.0.0 |

## Directory Structure

```
├── swish
│   ├── scripts
│   │   ├── gen_data.py         // Script for generating input data and ground truth data (supports Swish/Silu modes)
│   ├── CMakeLists.txt          // Build project file (supports -DUSE_SILU_MODE)
│   ├── data_utils.h            // Data read/write functions
│   ├── swish.asc               // Ascend C operator implementation & invocation example (two modes merged)
│   └── README.md               // Example documentation
```

## Example Description

- Example function:
  This example performs element-wise Swish/Silu activation computation on the input tensor and writes the computation result to the output tensor. Swish and Silu have a close mathematical relationship; Silu is a special case of Swish when beta=1.

  The computation formulas are as follows:
  $$dstLocal_i = Swish(srcLocal_i) = \frac{srcLocal_i}{1 + e^{-\beta \cdot srcLocal_i}}$$
  $$dstLocal_i = Silu(srcLocal_i) = \frac{srcLocal_i}{1 + e^{-srcLocal_i}}$$

  In this example, the Swish mode uses beta=1.702 (GELU approximation value), and the two modes are switched through the compilation macro `USE_SILU_MODE`.

- Example specifications:

| Example Type (OpType) | swish / silu |  |  |  |
| --- | --- | --- | --- | --- |
| Example Input |  |  |  |  |
| name | shape | data type | format |  |
| src | [1, 32] | float | ND |  |
| Example Output |  |  |  |  |
| dst | [1, 32] | float | ND |  |
| Kernel Function Name | swish_custom |  |  |  |

- Example implementation:
  This example implements a swish_custom example with fixed shapes of input src[1, 32] and output dst[1, 32].

  - Kernel implementation
    Core computation steps: After transferring the input data in, call the corresponding high-level API to complete the computation, then transfer the result out.

    Swish mode invocation:
    ```cpp
    AscendC::Swish(dstLocal, srcLocal, dataSize, scalarValue);
    ```

    Silu mode invocation:
    ```cpp
    AscendC::Silu<T, false>(dstLocal, srcLocal, dataSize);
    ```

  - Tiling implementation
    This example is a single-core element-wise computation scenario with no complex multi-core logic. The host side obtains the temporary buffer size required by the API through `AscendC::GetSwishTmpSize` (Swish mode) or `AscendC::GetSiluTmpSize` (Silu mode) and passes it directly to the kernel.

  - Invocation implementation
    Use the kernel invocation operator `<<<>>>` to call the kernel function.

## Build and Run

Run the following steps in the root directory of this example to build and run the example.

- Configure environment variables

  Configure environment variables based on the [installation method](../../../../../docs/en/quick_start.md#prepare&install) of the CANN development kit in the current environment.
  ```bash
  source ${install_path}/cann/set_env.sh
  ```

  > **Note:** `${install_path}` is the CANN package installation directory. When no installation directory is specified, the default installation path is `/usr/local/Ascend`.

- Run the example

  **Swish mode (default)**:
  ```bash
  mkdir -p build && cd build;
  cmake -DCMAKE_ASC_ARCHITECTURES=dav-2201 ..; make -j;
  python3 ../scripts/gen_data.py
  ./demo
  ```

  **Silu mode**:
  ```bash
  mkdir -p build && cd build;
  cmake -DUSE_SILU_MODE=ON -DCMAKE_ASC_ARCHITECTURES=dav-2201 ..; make -j;
  python3 ../scripts/gen_data.py --silu-mode
  ./demo
  ```

  To use CPU debugging or NPU simulation mode, add the `-DCMAKE_ASC_RUN_MODE=cpu` or `-DCMAKE_ASC_RUN_MODE=sim` parameter.

  For example:
  ```bash
  cmake -DCMAKE_ASC_RUN_MODE=cpu -DCMAKE_ASC_ARCHITECTURES=dav-2201 ..; make -j; # CPU debugging mode
  cmake -DCMAKE_ASC_RUN_MODE=sim -DCMAKE_ASC_ARCHITECTURES=dav-2201 ..; make -j; # NPU simulation mode
  ```

  > **Notice:** Before switching the build mode, clear the cmake cache by running `rm CMakeCache.txt` in the build directory and then re-run cmake.

- Build option description



| Option | Values | Description |
| --- | --- | --- |
| CMAKE_ASC_RUN_MODE | npu (default), cpu, sim | Run mode: NPU run, CPU debug, NPU simulation |
| CMAKE_ASC_ARCHITECTURES | dav-2201 (default), dav-3510 | NPU architecture: dav-2201 corresponds to Atlas A2 Training Series Products/Atlas A2 Inference Series Products and Atlas A3 Training Series Products/Atlas A3 Inference Series Products; dav-3510 corresponds to Ascend 950PR/Ascend 950DT |
| USE_SILU_MODE | OFF (default), ON | Example mode: OFF for Swish, ON for Silu |



- Execution result

  The following execution result indicates that the precision comparison is successful.
  ```bash
  test pass!
  ```
