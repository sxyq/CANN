# Where Example

## Overview

This example uses the Where high-level API to select elements from two source operands based on a specified condition. Both source operands can be LocalTensor or scalar.

## Supported Products and CANN Software Versions

| Product | CANN Software Version |
|------|-------------|
| Ascend 950PR/Ascend 950DT | >= CANN 9.1.0 |

## Directory Structure

```text
├── where
│   ├── scripts
│   │   └── gen_data.py         // Script for generating input data and ground truth data
│   ├── CMakeLists.txt          // Build project file
│   ├── data_utils.h            // Data read/write functions
│   ├── where.asc               // Ascend C example implementation & invocation example
│   └── README.md               // Example documentation
```

## Example Description

- Example function:
  Select elements from two source operands based on a specified condition to generate the destination operand. Both source operands can be LocalTensor or scalar.

  The computation formula is as follows:
  $$dst_i = \begin{cases}
  src0, & if condition \\
  src1, & otherwise
  \end{cases}$$

- Example specifications:

| Example Type (OpType) | where |  |  |  |
| --- | --- | --- | --- | --- |
| Example Input |  |  |  |  |
| name | shape | data type | format |  |
| src0 | [1, 32] | float | ND |  |
| src1 | [1, 32] | float | ND |  |
| condition | [1, 32] | bool | ND |  |
| Example Output |  |  |  |  |
| dst | [1, 32] | float | ND |  |
| Kernel Function Name | where_custom |  |  |  |

- Example implementation:
  This example implements a where_custom example with fixed shapes of input src0[1, 32], src1[1, 32], condition[1, 32], and output dst[1, 32].

  - Kernel implementation

    Use the Where high-level API to select elements from two source operands based on a condition, supporting mixed tensor and scalar modes.

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
