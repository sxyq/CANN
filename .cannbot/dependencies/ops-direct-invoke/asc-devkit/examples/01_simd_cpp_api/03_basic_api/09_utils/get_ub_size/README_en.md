# GetUBSize Example

## Overview

This example demonstrates the usage of the GetUBSizeInBytes and GetRuntimeUBSize interfaces for obtaining the maximum usable UB (Unified Buffer) size (in bytes). This example provides 2 different test scenarios.

| scenarioNum | API Interface | Description |
| --- | --- | --- |
| 1 | GetUBSizeInBytes | Returns a compile-time constant representing the maximum usable UB (Unified Buffer) size. For example, in the Ascend 950PR/Ascend 950DT scenario, the system reserves 8KB, UB totals 256KB, and returns 248KB |
| 2 | GetRuntimeUBSize | Returns a runtime variable representing the maximum usable UB (Unified Buffer) size. Applicable to SIMT and SIMD mixed programming scenarios, where SIMT scenarios reserve some space in UB for Dcache. For example, in the Ascend 950PR/Ascend 950DT scenario, SIMT programming allocates 32KB for Dcache, the system reserves 8KB, UB totals 256KB, and returns 216KB. In scenarios that do not use mixed SIMT and SIMD programming, it returns a fixed value of 248KB |

## Supported Products and CANN Versions

| Product | CANN Version |
|---------|-------------|
| Ascend 950PR/Ascend 950DT | >= CANN 9.1.0 |

## Directory Structure

```
├── get_ub_size
│   ├── scripts
│   │   ├── gen_data.py         // Script for generating input data and ground truth data
│   │   └── verify_result.py    // Script for verifying whether output data matches ground truth data
│   ├── CMakeLists.txt          // Build project file
│   ├── data_utils.h            // Data read/write functions
│   ├── get_ub_size.asc         // Ascend C example implementation & invocation example
│   └── README.md               // Example documentation
```

## Example Description

- Example function

  The example demonstrates functionality based on the Abs absolute value operation. Computation formula:
  ```
  z = Abs(x)
  ```

- Example specifications

  **Scenario 1: GetUBSizeInBytes**

| Category | name | shape | data type | format |
| --- | --- | --- | --- | --- |
| Example Input | x | [1, 16384] | half | ND |
| Example Output | z | [1, 16384] | half | ND |

  **Scenario 2: GetRuntimeUBSize**

| Category | name | shape | data type | format |
| --- | --- | --- | --- | --- |
| Example Input | x | [1, 126976] | half | ND |
| Example Output | z | [1, 126976] | half | ND |

- Example implementation

  - Kernel implementation

    - Call the GetUBSizeInBytes or GetRuntimeUBSize interface to obtain the usable UB (Unified Buffer) size for computing tileLength.
    - Call the DataCopy basic API to transfer data from GM (Global Memory) to UB (Unified Buffer).
    - Call the Abs interface to perform absolute value operation on the input tensor.
    - Call the DataCopy basic API to transfer computation results from UB (Unified Buffer) to GM (Global Memory).

  - Invocation implementation

    Use the kernel invocation operator <<<>>> to call the kernel function.

## Build and Run

Run the following steps in the root directory of this example to build and run the example.

- Configure environment variables

  Configure environment variables according to the [installation method](../../../../../docs/en/quick_start.md#prepare&install) of the CANN development kit in the current environment.
  ```bash
  source ${install_path}/cann/set_env.sh
  ```

  > **Note:** `${install_path}` is the CANN package installation directory. When no installation directory is specified, the default installation path is `/usr/local/Ascend`.

- Run the example

  Run the following commands in the example directory.
  ```bash
  SCENARIO_NUM=1
  mkdir -p build && cd build;      # Create and enter the build directory
  cmake -DSCENARIO_NUM=$SCENARIO_NUM -DCMAKE_ASC_ARCHITECTURES=dav-3510 ..;make -j;    # Build the project, default npu mode
  python3 ../scripts/gen_data.py -scenarioNum=$SCENARIO_NUM   # Generate test input data
  ./demo                           # Run the compiled executable to execute the example
  python3 ../scripts/verify_result.py output/output.bin output/golden.bin   # Verify whether the output result is correct and confirm algorithm logic is correct
  ```

  To use CPU debug or NPU simulation mode, add the `-DCMAKE_ASC_RUN_MODE=cpu` or `-DCMAKE_ASC_RUN_MODE=sim` parameter.

  Examples:
  ```bash
  cmake -DSCENARIO_NUM=$SCENARIO_NUM -DCMAKE_ASC_RUN_MODE=cpu -DCMAKE_ASC_ARCHITECTURES=dav-3510 ..;make -j; # CPU debug mode
  cmake -DSCENARIO_NUM=$SCENARIO_NUM -DCMAKE_ASC_RUN_MODE=sim -DCMAKE_ASC_ARCHITECTURES=dav-3510 ..;make -j; # NPU simulation mode
  ```

  > **Notice:** Clear the cmake cache before switching build modes. Run `rm CMakeCache.txt` in the build directory and then re-run cmake.

- Build option description

  | Option | Values | Description |
  |--------|--------|-------------|
  | `CMAKE_ASC_RUN_MODE` | `npu` (default), `cpu`, `sim` | Run mode: NPU execution, CPU debug, NPU simulation |
  | `CMAKE_ASC_ARCHITECTURES` | `dav-3510` (default) | NPU architecture: dav-3510 corresponds to Ascend 950PR/Ascend 950DT |
  | `SCENARIO_NUM` | `1`, `2` | Scenario number: 1=GetUBSizeInBytes, 2=GetRuntimeUBSize |

- Execution result

  The following execution result indicates that the accuracy comparison is successful:
  ```bash
  test pass!
  ```
