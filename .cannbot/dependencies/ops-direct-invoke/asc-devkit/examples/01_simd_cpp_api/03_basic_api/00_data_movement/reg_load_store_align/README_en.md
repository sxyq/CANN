# ld_st_reg_align Example

## Overview
This example implements contiguous and non-contiguous aligned data transfer operations between UB (Unified Buffer) and RegTensor (the basic unit of Reg vector computation) based on the Reg programming interface. This example uses the LoadAlign and StoreAlign APIs, as well as enabling modes such as POST_MODE_UPDATE and DATA_BLOCK_COPY. This example supports 6 transfer scenarios. Select a scenario through the CMake build parameter `SCENARIO_NUM`.

| SCENARIO_NUM | Transfer Scenario |
| --- | --- |
| 1 | Using developer-defined inter-iteration offset |
| 2 | Using PostUpdate mode for inter-iteration offset |
| 3 | Using address register (AddrReg) for inter-iteration offset |
| 4 | Non-contiguous transfer by DataBlock unit |
| 5 | Broadcast mode load |
| 6 | Upsample mode load |

## Supported Products and CANN Versions

| Product | CANN Version |
|---------|-------------|
| Ascend 950PR/Ascend 950DT | >= CANN 9.2.0 |

## Directory Structure
```
├── reg_load_store_align
│   ├── scripts
│   │   ├── gen_data.py                // Input data and ground truth generation script
│   ├── CMakeLists.txt                 // Build configuration file
│   ├── data_utils.h                   // Data read/write functions
│   ├── ld_st_reg_align.asc            // Ascend C example implementation & invocation example
│   └── README.md                      // Example description
```

## Example Description
**Scenario 1: Using Developer-defined Inter-iteration Offset**
- Example functionality:
  Reconfigures the load/store addresses each iteration to achieve contiguous data load/store. Also introduces how to handle the non-VL-aligned scenario where the last iteration has less data than VL using MaskReg.
- Example implementation:
  - Basic scenario. LoadAlign and StoreAlign use default parameter configuration.
  - Inputs 1024 elements, outputs 1021 elements, requiring 8 iterations. The last iteration only computes and stores 125 elements, which is less than VL.
  - Inside the VF function for loop, each iteration uses the UpdateMask API to update the mask. For half type:
    - When outputLength>=128, the mask handles 128 elements
    - When outputLength<128, the mask handles outputLength elements
    - After UpdateMask completes, outputLength decrements by the number of elements handled by the mask.
    ```cpp
    mask = AscendC::Reg::UpdateMask<T>(outputLength);
    ```
- Example specifications:

| Example Type (OpType) | AIV Example |  |  |
| --- | --- | --- | --- |
| Example Input | name | shape | data type |
| x | [1, 1024] | half |  |
| y | [1, 1024] | half |  |
| Example Output | z | [1, 1021] | half |
| VF Function Name | CopyWithOffsetVF |  |  |  |

**Scenario 2: Using PostUpdate Mode for Inter-iteration Offset**
- Example functionality:
  Load/store uses POST_MODE_UPDATE mode to achieve contiguous data load/store.
- Example implementation:
  - LoadAlign/StoreAlign template parameter postMode = PostLiteral::POST_MODE_UPDATE
  - LoadAlign/StoreAlign parameter postUpdateStride = 128, that is, VL/sizeof(half). Taking LoadAlign load as an example:
    - The UB start address for each iteration is srcAddr
    - After LoadAlign executes, srcAddr is automatically updated to srcAddr+postUpdateStride.
- Example specifications:

| Example Type (OpType) | AIV Example |  |  |
| --- | --- | --- | --- |
| Example Input | name | shape | data type |
| x | [1, 1024] | half |  |
| y | [1, 1024] | half |  |
| Example Output | z | [1, 1024] | half |
| VF Function Name | CopyWithPostUpdateVF |  |  |  |

**Scenario 3: Using Address Register (AddrReg) for Inter-iteration Offset**
- Example functionality:
  Load/store uses AddrReg (address register) to achieve contiguous data load/store.
- Example implementation:
  - Initialize the address register, indicating that aReg increments by stride0 when the i-axis loop completes
    ```cpp
    uint32_t stride0 = AscendC::GetVecLen() / sizeof(T);
    AddrReg aReg = AscendC::Reg::CreateAddrReg<T>(i, stride0);
    ```
  - LoadAlign/StoreAlign parameter offset = aReg. Taking LoadAlign load as an example:
    - The UB start address for each iteration is srcAddr + aReg
    - After the iteration, aReg is automatically updated to aReg + stride0.
- Example specifications:

| Example Type (OpType) | AIV Example |  |  |
| --- | --- | --- | --- |
| Example Input | name | shape | data type |
| x | [1, 1024] | half |  |
| y | [1, 1024] | half |  |
| Example Output | z | [1, 1024] | half |
| VF Function Name | CopyWithAddrRegVF |  |  |  |

**Scenario 4: Non-contiguous Transfer by DataBlock Unit**
- Example functionality:
  Load/store uses DataBlock (32 bytes) transfer mode. During load, each single transfer has an interval of 2 DataBlocks between adjacent DataBlocks, that is, transfers 32B with a 32B gap; during store, each single transfer has an interval of 1 DataBlock between adjacent DataBlocks, equivalent to contiguous transfer.
- Example implementation:
  - LoadAlign/StoreAlign template parameter dataMode = DataCopyMode::DATA_BLOCK_COPY
  - LoadAlign parameter dataBlockStride is configured as 2, StoreAlign parameter dataBlockStride is configured as 1
  - Each repeat processes 256B, that is, 8 DataBlocks, so LoadAlign parameter repeatStride is configured as 16, StoreAlign parameter repeatStride is configured as 8
- Example specifications:

| Example Type (OpType) | AIV Example |  |  |
| --- | --- | --- | --- |
| Example Input | name | shape | data type |
| x | [1, 1024] | half |  |
| y | [1, 1024] | half |  |
| Example Output | z | [1, 512] | half |
| VF Function Name | CopyDataBlock |  |  |  |

**Scenario 5: Broadcast Mode Load**
- Example functionality:
  Load uses broadcast mode. Each iteration loads the first element at the UB start address and broadcasts it to all element positions of regTensor.
- Example implementation:
  - LoadAlign template parameter dist = LoadDist::DIST_BRC_B16
- Example specifications:

| Example Type (OpType) | AIV Example |  |  |
| --- | --- | --- | --- |
| Example Input | name | shape | data type |
| x | [1, 1024] | half |  |
| y | [1, 1024] | half |  |
| Example Output | z | [1, 1024] | half |
| VF Function Name | CopyInBroadcastVF |  |  |  |

**Scenario 6: Upsample Mode Load**
- Example functionality:
  Load uses upsample mode. Each iteration loads VL/2 data, with each input element repeated twice.
- Example implementation:
  - LoadAlign template parameter dist = LoadDist::DIST_US_B16
- Example specifications:

| Example Type (OpType) | AIV Example |  |  |
| --- | --- | --- | --- |
| Example Input | name | shape | data type |
| x | [1, 1024] | half |  |
| y | [1, 1024] | half |  |
| Example Output | z | [1, 2048] | half |
| VF Function Name | CopyInUpsampleVF |  |  |  |

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
  SCENARIO_NUM=1                                                                # Execute scenario 1
  mkdir -p build && cd build;                                                   # Create and enter the build directory
  cmake -DSCENARIO_NUM=$SCENARIO_NUM -DCMAKE_ASC_ARCHITECTURES=dav-3510 ..;make -j; # Build the project (default npu mode)
  python3 ../scripts/gen_data.py -scenarioNum $SCENARIO_NUM                     # Generate test input data
  ./demo                                                                        # Run the compiled executable to execute the example
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
  | `CMAKE_ASC_RUN_MODE` | `npu` (default), `cpu`, `sim` | Run mode: NPU execution, CPU debug, NPU simulation |
  | `CMAKE_ASC_ARCHITECTURES` | `dav-3510` | NPU architecture: dav-3510 corresponds to Ascend 950PR/Ascend 950DT |
  | `SCENARIO_NUM` | `1`, `2`, `3`, `4`, `5`, `6` | Scenario number, see overview for details |

- Execution result
  The following execution result indicates that the precision comparison is successful.
  ```bash
  test pass!
  ```
