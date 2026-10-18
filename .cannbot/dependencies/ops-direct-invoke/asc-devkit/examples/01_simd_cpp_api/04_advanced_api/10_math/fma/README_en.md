# Fma Example

## Overview

This example uses the Fma high-level API to compute the result of multiplying two inputs and adding a third input element-wise.

## Supported Products and CANN Software Versions

| Product | CANN Software Version |
|------|-------------|
| Ascend 950PR/Ascend 950DT | >= CANN 9.1.0 |

## Directory Structure

```text
├── fma
│   ├── scripts
│   │   └── gen_data.py         // Script for generating input data and ground truth data
│   ├── CMakeLists.txt          // Build project file
│   ├── data_utils.h            // Data read/write functions
│   ├── fma.asc                 // Ascend C example implementation & invocation example
│   └── README.md               // Example documentation
```

## Example Description

- Example function:
  Compute the result of multiplying two inputs and adding a third input element-wise.

  The computation formulas are as follows:
  $$
  dst_i = Fma(src0_i, src1_i, src2_i)
  $$

  $$
  Fma(src0_i, src1_i, src2_i) = src0_i * src1_i + src2_i
  $$

- Example specifications:

| Example Type (OpType) | fma |  |  |  |
| --- | --- | --- | --- | --- |
| Example Input |  |  |  |  |
| name | shape | data type | format |  |
| src0 | [1, 128] | float | ND |  |
| src1 | [1, 128] | float | ND |  |
| src2 | [1, 128] | float | ND |  |
| Example Output |  |  |  |  |
| dst | [1, 128] | float | ND |  |
| Kernel Function Name | fma_custom |  |  |  |

- Example implementation:
  This example implements an fma_custom example with fixed shapes of input src0[1, 128], src1[1, 128], src2[1, 128], and output dst[1, 128].

  - Kernel implementation

    Use the Fma high-level API to compute src0 * src1 + src2, with optional use of a temporary buffer.

  - Tiling implementation

    The host side obtains the maximum and minimum temporary space required by the Fma API through GetFmaMaxMinTmpSize.

  - Invocation implementation
    Use the kernel invocation operator <<<>>> to call the kernel function.

## Build and Run

Run the following steps in the root directory of this example to build and run the example.

- Configure environment variables
  Configure environment variables based on the [installation method](../../../../../docs/en/quick_start.md#prepare&install) of the CANN development kit in the current environment.
  ```bash
  source ${install_path}/cann/set_env.sh
  ```

  > **Note:** `${install_path}` is the CANN package installation directory. When no installation directory is specified, the default installation path is `/usr/local/Ascend`.

- Run the example

  Run the following commands in the example directory.
  ```bash
  mkdir -p build && cd build;      # Create and enter the build directory
  cmake -DCMAKE_ASC_ARCHITECTURES=dav-3510 ..;make -j;    # Build the project, NPU mode by default
  python3 ../scripts/gen_data.py   # Generate test input data
  ./demo                           # Run the compiled executable to execute the example
  ```

  To use CPU debugging or NPU simulation mode, add the `-DCMAKE_ASC_RUN_MODE=cpu` or `-DCMAKE_ASC_RUN_MODE=sim` parameter.

  For example:

  ```bash
  cmake -DCMAKE_ASC_RUN_MODE=cpu -DCMAKE_ASC_ARCHITECTURES=dav-3510 ..;make -j; # CPU debugging mode
  cmake -DCMAKE_ASC_RUN_MODE=sim -DCMAKE_ASC_ARCHITECTURES=dav-3510 ..;make -j; # NPU simulation mode
  ```

  > **Notice:** Before switching the build mode, clear the cmake cache by running `rm CMakeCache.txt` in the build directory and then re-run cmake.

- Build option description

  | Option | Values | Description |
  |------|--------|------|
  | `CMAKE_ASC_RUN_MODE` | `npu` (default), `cpu`, `sim` | Run mode: NPU run, CPU debug, NPU simulation |
  | `CMAKE_ASC_ARCHITECTURES` | `dav-3510` (default) | NPU architecture: dav-3510 corresponds to Ascend 950PR/Ascend 950DT |

- Execution result

  The following execution result indicates that the precision comparison is successful.

  ```bash
  test pass!
  ```
