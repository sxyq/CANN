# move_reg Example

## Overview
This example demonstrates and verifies data transfer from a RegTensor (the basic unit of Reg vector computation) to a MaskReg (mask register) based on the Reg programming interface, using the MaskGenWithRegTensor API. It supports multiple scenarios. Select a scenario through the CMake build parameter `SCENARIO_NUM`.

| SCENARIO_NUM | Scenario Type |
| --- | --- |
| 1 | Downsampling scenario: Calls MaskGenWithRegTensor to extract data from RegTensor and fill it into MaskReg |
| 2 | Composite computation scenario: Calls MaskGenWithRegTensor to load MaskReg, then uses it as a mask for Select to complete selection computation |

## Supported Products and CANN Versions

| Product | CANN Version |
|---------|-------------|
| Ascend 950PR/Ascend 950DT | >= CANN 9.2.0 |

## Directory Structure
```
├── reg_move
│   ├── scripts
│   │   └── gen_data.py                // Input data and ground truth generation script
│   ├── figures                        // Illustrations
│   ├── CMakeLists.txt                 // Build configuration file
│   ├── data_utils.h                   // Data read/write functions
│   ├── README.md                      // Example description
│   ├── move_reg_scenario_1.asc        // Scenario 1: Ascend C implementation for downsampling scenario
│   └── move_reg_scenario_2.asc        // Scenario 2: Ascend C implementation for composite computation scenario
```

## Example Description

### Scenario 1: Downsampling Scenario
- Example functionality:
  Demonstrates and verifies data transfer from regTensor to maskReg. Inputs 64 int32-type cond data elements, uses MaskGenWithRegTensor<int32, 0> to extract 64 bits of data from the first 8 bytes of condReg, fills them into a 256-bit MaskReg in downsampling mode (every 4 mask bits correspond to 1 src bit), and outputs 32 bytes (256 bits) of mask data to UB (Unified Buffer).

  - Example specifications:

| Example Type (OpType) | AIV Example |  |  |
| --- | --- | --- | --- |
| Example Input | name | shape | data type |
| cond | [1, 64] | int32 |  |
| Example Output | maskOut | [32] | uint8 |
| Kernel Function Name | move_reg_scenario_1 |  |  |

- Example implementation:
  1. Uses LoadAlign to load cond data into RegTensor with alignment
  2. Through MaskGenWithRegTensor<int32, 0>, extracts 64 bits of data from the first 8 bytes of condReg, fills them into a 256-bit MaskReg in downsampling mode: every 4 mask bits correspond to 1 src bit. Float element processing diagram:
  <img src="figures/move_reg_1.png">
  3. Uses StoreAlign to write mask data back to UB
- Invocation implementation
  Uses the kernel invocation syntax `<<<>>>` to call the kernel function, launching 1 core.

### Scenario 2: Composite Computation Scenario
- Example functionality:
  Selects values from x or y at corresponding positions based on the bits in mask. When a mask bit is 1, the corresponding element from x is selected; when a mask bit is 0, the corresponding element from y is selected.
- Example specifications:

| Example Type (OpType) | AIV Example |  |  |
| --- | --- | --- | --- |
| Example Input | name | shape | data type |
| x | [1, 256] | float |  |
| y | [1, 256] | float |  |
| mask | [1, 8] | float |  |
| Example Output | z | [1, 256] | float |
| Kernel Function Name | move_reg_scenario_2 |  |  |  |

- Example implementation:
  1. Calls LoadAlign to load selection operands x, y into RegTensor
  2. Calls LoadUnAlignPre/LoadUnAlign to load the mask operand into RegTensor, loading 2 elements per iteration, that is, 8 bytes / 64 bits of data
  3. Calls MaskGenWithRegTensor to extract 64 bits of data from the first 8 bytes of mReg, fills them into a 256-bit MaskReg in downsampling mode: every 4 mask bits correspond to 1 src bit. Processing diagram:
  <img src="figures/move_reg_1.png">
  4. Calls the Select API to select elements from xReg or yReg element-wise based on maskReg, writing into zReg, as shown below
  <img src="figures/move_reg_2.png">
  5. Calls StoreAlign to write Select data back to UB

- Invocation implementation
  Uses the kernel invocation syntax <<<>>> to call the kernel function.

## Build and Run
Run the following steps in the root directory of this example to build and run it.
- Configure environment variables
  Configure environment variables based on the [installation method](../../../../../docs/en/quick_start.md#prepare&install) of the CANN development kit on the current environment.
  ```bash
  source ${install_path}/cann/set_env.sh
  ```

  > **Note:** `${install_path}` is the CANN package installation directory. When no installation directory is specified, the default installation path is `/usr/local/Ascend`.

- Run the example

  Run the following commands in the example directory.
  ```bash
  SCENARIO_NUM=1                                                                # Set the scenario number
  mkdir -p build && cd build;                                                   # Create and enter the build directory
  cmake -DCMAKE_ASC_ARCHITECTURES=dav-3510 -DSCENARIO_NUM=$SCENARIO_NUM ..;make -j; # Build the project (default npu mode)
  python3 ../scripts/gen_data.py -scenarioNum=$SCENARIO_NUM                     # Generate test input data
  ./demo                                                                        # Run the compiled executable to execute the example
  ```

  To use CPU debug or NPU simulation mode, add the `-DCMAKE_ASC_RUN_MODE=cpu` or `-DCMAKE_ASC_RUN_MODE=sim` parameter.

  Examples:
  ```bash
  cmake -DCMAKE_ASC_RUN_MODE=cpu -DCMAKE_ASC_ARCHITECTURES=dav-3510 -DSCENARIO_NUM=$SCENARIO_NUM ..;make -j; # CPU debug mode
  cmake -DCMAKE_ASC_RUN_MODE=sim -DCMAKE_ASC_ARCHITECTURES=dav-3510 -DSCENARIO_NUM=$SCENARIO_NUM ..;make -j; # NPU simulation mode
  ```

  > **Notice:** Clear the cmake cache before switching build modes or scenarios. Run `rm CMakeCache.txt` in the build directory and re-run cmake.

- Build option description

  | Option | Values | Description |
  |--------|--------|-------------|
  | `CMAKE_ASC_RUN_MODE` | `npu` (default), `cpu`, `sim` | Run mode: NPU execution, CPU debug, NPU simulation |
  | `CMAKE_ASC_ARCHITECTURES` | `dav-3510` | NPU architecture: dav-3510 corresponds to Ascend 950PR/Ascend 950DT |
  | `SCENARIO_NUM` | `1`, `2` | Scenario number: 1=downsampling scenario, 2=composite computation scenario |

- Execution result
  The following execution result indicates that the precision comparison is successful.
  ```bash
  test pass!
  ```
