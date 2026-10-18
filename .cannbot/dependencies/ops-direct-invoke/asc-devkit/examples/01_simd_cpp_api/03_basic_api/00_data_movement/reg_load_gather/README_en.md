# reg_load_gather Example

## Overview
This example implements the functionality of transferring discrete data from UB (Unified Buffer) into a RegTensor (the basic unit of Reg vector computation) based on the Reg programming interface. It supports two scenarios. Select a scenario through the CMake build parameter `SCENARIO_NUM`.

| SCENARIO_NUM | Load Scenario |
| --- | --- |
| 1 | Gather (collect single-point data from UB to RegTensor by index) |
| 2 | GatherB (collect DataBlock-sized data from UB to RegTensor by index) |

## Supported Products and CANN Versions

| Product | CANN Version |
|---------|-------------|
| Ascend 950PR/Ascend 950DT | >= CANN 9.2.0 |

## Directory Structure
```
├── reg_load_gather
│   ├── scripts
│   │   ├── gen_data.py                // Input data and ground truth generation script
│   ├── CMakeLists.txt                 // Build configuration file
│   ├── data_utils.h                   // Data read/write functions
│   ├── gather_ld_reg.asc              // Ascend C example implementation & invocation example
│   └── README.md                      // Example description
```

## Example Description
- Example functionality:
  Collects elements discretely from source data to the destination address based on indices.

  **Scenario 1: Gather Mode**
  - Uses the Gather API to collect data from UB to RegTensor by element index
  - Collects by element: dst[i] = src[index[i]]
  - Source data volume is 1024 elements, index data volume is 128 elements, output data volume is 128 elements.
  - Example specifications:

| Example Type (OpType) | AIV Example |  |  |
| --- | --- | --- | --- |
| Example Input | name | shape | data type |
| src | [1, 1024] | half |  |
| index | [1, 128] | uint16_t |  |
| Example Output | dst | [1, 128] | half |
| Kernel Function Name | gather_ld_reg |  |  |

  - Example implementation:
    The GatherVF function first loads index data into indexReg via LoadAlign, then calls the Gather API to collect data from the source address by index into dstReg, and finally writes the result back to UB via StoreAlign.
    - Invocation implementation
      Uses the kernel invocation syntax `<<<>>>` to call the kernel function, launching 1 core.

  **Scenario 2: GatherB Mode**
  - Uses the GatherB API to collect by 32-byte DataBlock
  - Source data volume is 1024 elements, index data volume is 8 elements (corresponding to 8 DataBlocks), output data volume is 128 elements.
  - Example specifications:

| Example Type (OpType) | AIV Example |  |  |
| --- | --- | --- | --- |
| Example Input | name | shape | data type |
| src | [1, 1024] | half |  |
| index | [1, 8] | uint32_t |  |
| Example Output | dst | [1, 128] | half |
| Kernel Function Name | gather_ld_reg |  |  |

  - Example implementation:
    The GatherBVF function first loads index data into indexReg via LoadAlign, then calls the GatherB API to collect data from the source address by DataBlock into dstReg, and finally writes the result back to UB via StoreAlign.
    - Invocation implementation
      Uses the kernel invocation syntax `<<<>>>` to call the kernel function, launching 1 core.

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
  SCENARIO_NUM=1                                                                 # Select the execution scenario (1=Gather, 2=GatherB)
  mkdir -p build && cd build;                                                    # Create and enter the build directory
  cmake -DSCENARIO_NUM=$SCENARIO_NUM -DCMAKE_ASC_ARCHITECTURES=dav-3510 ..;make -j;  # Build the project (default npu mode)
  python3 ../scripts/gen_data.py -scenarioNum $SCENARIO_NUM                      # Generate test input data
  ./demo                                                                         # Run the compiled executable to execute the example
  ```

  To use CPU debug or NPU simulation mode, add the `-DCMAKE_ASC_RUN_MODE=cpu` or `-DCMAKE_ASC_RUN_MODE=sim` parameter.

  Examples:
  ```bash
  cmake -DSCENARIO_NUM=$SCENARIO_NUM -DCMAKE_ASC_RUN_MODE=cpu -DCMAKE_ASC_ARCHITECTURES=dav-3510 ..;make -j; # CPU debug mode
  cmake -DSCENARIO_NUM=$SCENARIO_NUM -DCMAKE_ASC_RUN_MODE=sim -DCMAKE_ASC_ARCHITECTURES=dav-3510 ..;make -j; # NPU simulation mode
  ```

  > **Notice:** Clear the cmake cache before switching build modes or scenarios. Run `rm CMakeCache.txt` in the build directory and re-run cmake.

- Build option description

  | Option | Values | Description |
  |--------|--------|-------------|
  | `SCENARIO_NUM` | 1, 2 | Example execution scenario: Scenario 1=Gather mode, Scenario 2=GatherB mode |
  | `CMAKE_ASC_RUN_MODE` | `npu` (default), `cpu`, `sim` | Run mode: NPU execution, CPU debug, NPU simulation |
  | `CMAKE_ASC_ARCHITECTURES` | `dav-3510` | NPU architecture: dav-3510 corresponds to Ascend 950PR/Ascend 950DT |

- Execution result

  The following execution result indicates that the precision comparison is successful.
  ```bash
  test pass!
  ```
