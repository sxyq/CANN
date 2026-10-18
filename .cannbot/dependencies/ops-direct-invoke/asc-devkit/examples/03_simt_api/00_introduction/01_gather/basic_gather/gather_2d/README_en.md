# SIMT Programming Mode Implementation of 2D Gather Operator Sample

## Overview

This sample implements a simple scenario (fixed shape) 2D Gather operator using the Ascend C SIMT programming mode. It collects specified m rows of data from the input tensor, demonstrating the development method for discrete memory access operators in simplified scenarios.

## Supported Products

- Ascend 950PR/Ascend 950DT

## Supported CANN Software Versions

- \>= CANN 9.1.0

## Directory Structure

```text
├── gather_2d
│   ├── CMakeLists.txt            # cmake build file
│   ├── gather_2d.asc             # SIMT implementation of 2D gather call sample
│   └── README.md
```

## Operator Description

- Operator Function:
  The gather_2d operator retrieves 12288 rows of data at specified indices from a 2D vector with shape [100000,128]. The calculation formula for the i-th row of the operator output is:

  ```text
  output[i] = input[index[i]]
  ```

- Operator Specification:

| Operator Type(OpType) | gather_2d |  |  |  |
| --- | --- | --- | --- | --- |
| Operator Input | name | shape | data type | format |
| input | [100000,128] | float | ND |  |
| index | [12288] | uint32_t | ND |  |
| Operator Output | output | [12288,128] | float | ND |
| Kernel Function Name | gather_2d_custom |  |  |  |

- Data Partitioning:
  * Number of cores: 48 cores
  * Threads per core: 256 threads
  * Processing per thread: 1 row (128 columns)
  * Total processing capacity: 48×256=12288 rows (covering index length)

- Operator Implementation:
  The gather_2d operator implementation process retrieves data at specified indices from the input (Global Memory). Based on the data partitioning above, it first calculates the index of data that the thread should process, then stores one row of data to Global Memory through an assignment operation.

- Invocation Implementation:
  Use the kernel call operator <<<>>> to invoke the kernel function.

## Compilation and Execution

Execute the following steps in the root directory of this sample to compile and run the operator.

- Configure Environment Variables
  Configure the environment variables according to the [installation method](../../../../../../docs/en/quick_start.md#prepare&install) of the CANN development kit package on the current environment.
  ```bash
  source ${install_path}/cann/set_env.sh
  ```

  > **Note:** `${install_path}` is the CANN package installation directory. When no installation directory is specified, it defaults to `/usr/local/Ascend`.

- Sample Execution

  Execute the following commands in this sample directory.

  ```bash
  mkdir -p build && cd build;   # Create and enter build directory
  cmake -DCMAKE_ASC_ARCHITECTURES=dav-3510 ..; make -j;   # Build the project
  ./demo                        # Run the sample
  ```

  When using NPU simulation mode, add the `-DCMAKE_ASC_RUN_MODE=sim` parameter.

  Example:
  ```bash
  cmake -DCMAKE_ASC_RUN_MODE=sim -DCMAKE_ASC_ARCHITECTURES=dav-3510 ..; make -j;   # NPU simulation mode
  ```

  > **Note:** Clear the cmake cache before switching build modes. Execute `rm CMakeCache.txt` in the build directory, then run cmake again.

  Compilation Option Description

  | Option | Available Values | Description |
  |------|--------|------|
  | `CMAKE_ASC_RUN_MODE` | `npu` (default), `sim` | Run mode: NPU execution or NPU simulation |
  | `CMAKE_ASC_ARCHITECTURES` | `dav-3510` | NPU Architecture: This sample only supports dav-3510 (Ascend 950PR/Ascend 950DT) |

  The execution result is as follows, indicating that the accuracy comparison passed.

  ```text
  [Success] Case accuracy is verification passed.
  ```
