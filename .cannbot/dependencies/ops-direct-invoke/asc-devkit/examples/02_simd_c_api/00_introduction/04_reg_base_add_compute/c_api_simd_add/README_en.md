# Add Operator Sample Using C_API (RegBase Scenario)

## Overview

This sample implements the Add operator using the C_API interface. It is based on synchronous data movement and computation interfaces.

## Supported Products

- Ascend 950PR/Ascend 950DT

## Directory Structure

```
├── c_api_simd_add
│   ├── CMakeLists.txt          // Build project file
│   ├── c_api_add.asc           // Ascend C operator implementation and invocation sample
│   └── README.md
```

## Operator Description

- Operator Function:
  The Add operator adds two data elements and returns the result. The corresponding mathematical expression is:

  ```
  z = x + y
  ```

- Operator Specification:

| Operator Type(OpType) | Add |  |  |  |
| --- | --- | --- | --- | --- |
| Operator Input | name | shape | data type | format |
| x | 2048*8 | float | ND |  |
| y | 2048*8 | float | ND |  |
| Operator Output | z | 2048*8 | float | ND |
| Kernel Function Name | add_custom |  |  |  |

- Operator Implementation:

  - Kernel Implementation

    C_API input data must first be moved to on-chip memory, then loaded into Reg vector computation registers. After that, the computation interface adds the two input parameters to obtain the result, which is then moved to Local Memory and finally moved to external memory.

    The Add operator implementation process consists of three steps:

    Step 1: Move the inputs x and y from Global Memory to Local Memory, storing them in xLocal and yLocal respectively.

    Step 2: Load data from Unified Buffer to registers src0 and src1, use `asc_add` to perform addition on the register data, store the result in register dst, and move the result back to zLocal after computation completes.

    Step 3: Move the output data from zLocal to the output z in Global Memory.

  - Mask Control in Vector Computation

    In vector computation instructions, mask controls which channels of the vector register participate in computation. This sample uses the asc_update_mask_b32 function to set a 32-bit vector mask, controlling the validity of each channel in the 256-bit vector register.

  - Invocation Implementation
    Use the kernel invocation operator <<<>>> to call the kernel function.

## Build and Run

Execute the following steps in the sample root directory to build and run the operator.

- Configure Environment Variables
  Select the appropriate command to configure environment variables based on the [installation method](../../../../../docs/en/quick_start.md#prepare&install) of the CANN development toolkit on your current environment.
  - Default path, CANN package installed by root user

    ```bash
    source /usr/local/Ascend/cann/set_env.sh
    ```

  - Default path, CANN package installed by non-root user

    ```bash
    source $HOME/Ascend/cann/set_env.sh
    ```

  - Specified path install_path, CANN package installation

    ```bash
    source ${install_path}/cann/set_env.sh
    ```

- Sample Execution

  ```bash
  mkdir -p build && cd build;   # Create and enter build directory
  cmake ..;make -j;             # Build project
  # Execute the following in the build directory
  ./c_api_add_example           # Run sample
  ```

  The following execution result indicates that the accuracy verification passed successfully.

  ```bash
  [Success] Case accuracy is verification passed.
  ```
