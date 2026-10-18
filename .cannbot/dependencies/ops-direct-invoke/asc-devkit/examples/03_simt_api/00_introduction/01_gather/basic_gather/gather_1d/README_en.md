# One-Dimensional Gather Operator Sample Using SIMT Programming Mode

## Overview

This sample implements a one-dimensional Gather operator in a simple scenario (with fixed shape) using the Ascend C SIMT programming mode. It collects elements at specified indices from a one-dimensional input, demonstrating the development method for discrete memory access operators in simplified scenarios.

## Supported Products

- Ascend 950PR/Ascend 950DT

## Supported CANN Software Versions

- \>= CANN 9.1.0

## Directory Structure

```text
├── gather_1d
│   ├── CMakeLists.txt         # CMake build file
│   ├── gather_1d.asc          # Sample for one-dimensional gather using SIMT
│   └── README.md
```

## Operator Description

- Operator Function:
  The gather_1d operator retrieves one data element from a one-dimensional input tensor with shape [100000] based on each element in the index. The formula for calculating the i-th element of the operator output is:

  ```text
  output[i] = input[index[i]]
  ```

- Operator Specification:

| Operator Type (OpType) | gather_1d |  |  |  |
| --- | --- | --- | --- | --- |
| Operator Input | name | shape | data type | format |
| input | [100000] | float | ND |  |
| index | [12288] | int32_t | ND |  |
| Operator Output | output | [12288] | float | ND |
| Kernel Function Name | gather_1d_custom |  |  |  |

- Data Partitioning:
  * Number of cores: 48 cores
  * Threads per core: 256 threads
  * Elements processed per thread: 1 element
  * Total processing capacity: 48 * 256 = 12288 elements
  * Expected result: The i-th element of the output equals the index[i]-th element of the input

- Operator Implementation:
  The implementation process of the gather_1d operator retrieves data at specified indices from the input (Global Memory). Based on the above data partitioning, it first calculates the index of data that the thread should process, then reads one element according to index[i] and writes it to output[i].

- Invocation Implementation:
  Use the kernel call operator `<<<>>>` to launch the kernel.

## Build and Run

Execute the following steps in the root directory of this sample to build and run the operator.

- Configure Environment Variables
  Configure the environment variables according to the [installation method](../../../../../../docs/en/quick_start.md#prepare&install) of the CANN development kit package on the current environment.
  ```bash
  source ${install_path}/cann/set_env.sh
  ```

  > **Note:** `${install_path}` is the CANN package installation directory. If no installation directory is specified, it defaults to `/usr/local/Ascend`.

- Sample Execution

  Execute the following commands in this sample directory.

  ```bash
  mkdir -p build && cd build;   # Create and enter the build directory
  cmake -DCMAKE_ASC_ARCHITECTURES=dav-3510 ..; make -j;   # Build the project
  ./demo                        # Run the sample
  ```

  When using NPU simulation mode, add the `-DCMAKE_ASC_RUN_MODE=sim` parameter.

  Example:
  ```bash
  cmake -DCMAKE_ASC_RUN_MODE=sim -DCMAKE_ASC_ARCHITECTURES=dav-3510 ..; make -j;   # NPU simulation mode
  ```

  > **Note:** Clear the cmake cache before switching build modes. Execute `rm CMakeCache.txt` in the build directory, then run cmake again.

  Build Option Description

  | Option | Available Values | Description |
  |------|--------|------|
  | `CMAKE_ASC_RUN_MODE` | `npu` (default), `sim` | Run mode: NPU execution, NPU simulation |
  | `CMAKE_ASC_ARCHITECTURES` | `dav-3510` | NPU architecture: This sample only supports dav-3510 (Ascend 950PR/Ascend 950DT) |

  The following execution result indicates that the accuracy comparison succeeds.

  ```text
  [Success] Case accuracy is verification passed.
  ```
