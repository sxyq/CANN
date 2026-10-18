# copy API and Matmul with Bias Example Based on Tensor API

## Overview

This example implements dynamic shape matrix multiplication with Bias based on the Tensor API programming approach, demonstrating the usage of copy (GM to L1, L1 to L0) transpose and non-transpose scenarios and mmad (matrix multiply-add) Tensor APIs. It supports selecting the transpose mode and data type through the build-time parameter SCENARIO_NUM. Shape parameters are dynamically passed at runtime through the MatmulTiling structure.

## Supported Products and CANN Versions

| Product | CANN Version |
|---------|-------------|
| Ascend 950PR/Ascend 950DT | > CANN 9.1.0 |

> **Note:** This example depends on CANN features that have not been officially released. Use the latest CANN master package.

## Directory Structure

```text
├── copy_in_tensor_api
│   ├── scripts
│   │   ├── gen_data.py                    // Input data and ground truth generation script
│   │   └── verify_result.py               // Ground truth comparison file
│   ├── CMakeLists.txt                     // Build configuration file
│   ├── data_utils.h                       // Data read/write functions
│   ├── copy_in_tensor_api.asc             // Ascend C example implementation & invocation example
│   ├── README_en.md                       // Example description document (English)
│   └── README.md                          // Example description document
```

## Example Description

- Example functionality:
  This example implements dynamic Shape multi-core Matmul with Bias, performing GM-to-L1 and L1-to-L0 data transfer through the copy API, and matrix multiply-add computation through the mmad API.

  1. Dynamic Matmul Functionality

  This example implements multi-core Matmul functionality. Shape parameters (M, N, K, singleCoreM, singleCoreN, singleCoreK) are dynamically passed to the kernel at runtime through the MatmulTiling structure. baseM, baseN, baseK and stepM, stepN, stepK serve as compile-time template parameters, determining the allocation size of L0/L1 level buffers.

  2. Bias Functionality

  This example adds the Bias vector to the matrix multiplication result through the mmad API in the first iteration of Matmul computation. Bias is a float-type one-dimensional vector with length N.

  3. Multi-scenario Support

  Select different transpose mode and data type combinations through the build-time macro parameter SCENARIO_NUM:

  | SCENARIO_NUM | Transpose Mode | A/B Data Type | GM Layout | L1 Layout | Description |
  | :--- | :--- | :--- | :--- | :--- | :--- |
  | 0 | AB not transposed | half | ND | NZ | AB matrices are not transposed, data type is half |
  | 1 | AB not transposed | float | ND | NZ | AB matrices are not transposed, data type is float |
  | 2 | AB transposed | half | DN | ZN | AB matrices are stored transposed, data type is half |
  | 3 | AB transposed | float | DN | ZN | AB matrices are stored transposed, data type is float |

  - Non-transpose mode (SCENARIO_NUM=0/1): Matrices A and B use ND/NZ layout in both GM and L1, with data stored in row-major order
  - Transpose mode (SCENARIO_NUM=2/3): Matrices A and B use DN/ZN layout in both GM and L1, with data stored after transposition

  Note: C matrix and Bias data types are float in all scenarios.

  4. MatmulTiling Structure Definition

  ```cpp
  struct MatmulTiling {
      int32_t m;            // Number of rows of matrix A
      int32_t n;            // Number of columns of matrix B
      int32_t k;            // Number of columns of matrix A / number of rows of matrix B
      int32_t singleCoreM;  // M dimension size processed per core
      int32_t singleCoreN;  // N dimension size processed per core
      int32_t singleCoreK;  // K dimension size processed per core
  };
  ```

- Example specifications:
  In the kernel function direct invocation example, the default supported shape is: M = 1024, N = 1024, K = 256.

  **Scenario 0: AB not transposed, half**


| Example Type (OpType) | Matmul with Bias |  |  |  |
| --- | --- | --- | --- | --- |
| Example Input | name | shape | data type | format |
| a (Matrix A) | [M, K] | half | ND |  |
| b (Matrix B) | [K, N] | half | ND |  |
| bias | [N] | float | ND |  |
| tiling | MatmulTiling structure | int32_t | ND |  |
| Example Output | c (Matrix C) | [M, N] | float | ND |
| Kernel Function Name | copy_in_tensor_api |  |  |  |

  **Scenario 1: AB not transposed, float**


| Example Type (OpType) | Matmul with Bias |  |  |  |
| --- | --- | --- | --- | --- |
| Example Input | name | shape | data type | format |
| a (Matrix A) | [M, K] | float | ND |  |
| b (Matrix B) | [K, N] | float | ND |  |
| bias | [N] | float | ND |  |
| tiling | MatmulTiling structure | int32_t | ND |  |
| Example Output | c (Matrix C) | [M, N] | float | ND |
| Kernel Function Name | copy_in_tensor_api |  |  |  |

  **Scenario 2: AB transposed, half**


| Example Type (OpType) | Matmul with Bias |  |  |  |
| --- | --- | --- | --- | --- |
| Example Input | name | shape | data type | format |
| a (Matrix A, transposed storage) | [K, M] | half | DN |  |
| b (Matrix B, transposed storage) | [N, K] | half | DN |  |
| bias | [N] | float | ND |  |
| tiling | MatmulTiling structure | int32_t | ND |  |
| Example Output | c (Matrix C) | [M, N] | float | ND |
| Kernel Function Name | copy_in_tensor_api |  |  |  |

  **Scenario 3: AB transposed, float**


| Example Type (OpType) | Matmul with Bias |  |  |  |
| --- | --- | --- | --- | --- |
| Example Input | name | shape | data type | format |
| a (Matrix A, transposed storage) | [K, M] | float | DN |  |
| b (Matrix B, transposed storage) | [N, K] | float | DN |  |
| bias | [N] | float | ND |  |
| tiling | MatmulTiling structure | int32_t | ND |  |
| Example Output | c (Matrix C) | [M, N] | float | ND |
| Kernel Function Name | copy_in_tensor_api |  |  |  |

  Compile-time template parameter default values:

  | Parameter | Default | Description |
  | :------ | :----- | :----------------------- |
  | BASE_M  | 128    | M dimension base block size |
  | BASE_N  | 128    | N dimension base block size |
  | BASE_K  | 64     | K dimension base block size |
  | STEP_M  | 1      | M dimension L1 cache step |
  | STEP_N  | 1      | N dimension L1 cache step |
  | STEP_K  | 4      | K dimension L1 cache step |

  Runtime dynamic parameter default values:

  | Parameter | Default | Description |
  | :----------- | :----- | :--------------------- |
  | m            | 1024   | Number of rows of matrix A |
  | n            | 1024   | Number of columns of matrix B |
  | k            | 256    | Number of columns of matrix A / number of rows of matrix B |
  | singleCoreM  | 256    | Per-core M dimension size |
  | singleCoreN  | 128    | Per-core N dimension size |
  | singleCoreK  | 256    | Per-core K dimension size |

## Build and Run

Run the following steps in the root directory of this example to build and run it.

- Configure environment variables
  Configure environment variables based on the [installation method](../../../../docs/en/quick_start.md#prepare&install) of the CANN development kit on the current environment. **Currently only [CANN master](../../../../docs/en/quick_start.md#cann-install) is supported.**

  ```bash
  source ${install_path}/cann/set_env.sh
  ```

  > **Note:** `${install_path}` is the CANN package installation directory. When no installation directory is specified, the default installation path is `/usr/local/Ascend`.

- Run the example
  Taking scenario 0 (AB not transposed, half) as an example:

  ```bash
  SCENARIO_NUM=0
  mkdir -p build && cd build;                                                    # Create and enter the build directory
  cmake -DSCENARIO_NUM=${SCENARIO_NUM} -DCMAKE_ASC_ARCHITECTURES=dav-3510 ..;make -j; # Build the project, default NPU mode
  python3 ../scripts/gen_data.py ${SCENARIO_NUM};                                    # Generate test input data for scenario 0
  ./demo;                                                                       # Run the compiled executable to execute the example
  python3 ../scripts/verify_result.py output/output.bin output/golden.bin        # Verify whether the output result is correct, confirming algorithm logic correctness
  ```

  To use NPU simulation mode, add the `-DCMAKE_ASC_RUN_MODE=sim` parameter. Example:

  ```bash
  cmake -DSCENARIO_NUM=${SCENARIO_NUM} -DCMAKE_ASC_RUN_MODE=sim -DCMAKE_ASC_ARCHITECTURES=dav-3510 ..;make -j; # NPU simulation mode
  ```

  > **Notice:** Clear the cmake cache before switching build modes. Run `rm CMakeCache.txt` in the build directory and re-run cmake.

  When building other scenarios, modify the `SCENARIO_NUM` variable. For example, to build scenario 2 (AB transposed, half), set `SCENARIO_NUM=2` and pass it to `gen_data.py` as well.

- Build option description

  | Option | Values | Description |
  |--------|--------|-------------|
  | `CMAKE_ASC_RUN_MODE` | `npu` (default), `sim` | Run mode: NPU execution, NPU simulation |
  | `CMAKE_ASC_ARCHITECTURES` | `dav-3510` | NPU architecture: dav-3510 corresponds to Ascend 950PR/Ascend 950DT |
  | `SCENARIO_NUM` | `0` (default), `1`, `2`, `3` | Scenario number: 0=AB not transposed half, 1=AB not transposed float, 2=AB transposed half, 3=AB transposed float |

- Execution result

  The following execution result indicates that the precision comparison is successful.

  ```bash
  test pass!
  ```
