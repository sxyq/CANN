# fixpipe_l0c2l1 Example

## Overview

This example introduces how to use Fixpipe to transfer matrix multiplication results from L0C (L0C Buffer) to L1 (L1 Buffer), supporting data type conversion, inline quantization, ReLU, and other features. These interfaces efficiently transfer matrix multiplication computation results from L0C to L1 Buffer, with support for various data format conversions and preprocessing capabilities.

Note:
- Atlas A3 Training Series Products/Atlas A3 Inference Series Products and Atlas A2 Training Series Products/Atlas A2 Inference Series Products only support Nz output format in the L0C to L1 path, and do not support float output data type; it must be quantized to other data types.
- Ascend 950PR/Ascend 950DT does not support direct data transfer from L1 to GM. Therefore, in this example, the result matrix transferred from L0C to L1 serves as input for the next matrix multiplication, performing another matrix computation and outputting the result to GM. (Atlas A2/A3 series products support direct data transfer from L1 to GM; this example chooses direct transfer out)

## Supported Products and CANN Versions

| Product | CANN Version |
|---------|-------------|
| Ascend 950PR/Ascend 950DT | >= CANN 9.2.0 |
| Atlas A3 Training Series Products/Atlas A3 Inference Series Products | >= CANN 9.2.0 |
| Atlas A2 Training Series Products/Atlas A2 Inference Series Products | >= CANN 9.2.0 |

## Directory Structure

```
├── fixpipe_l0c2l1
│   ├── scripts
│   │   ├── gen_data.py                // Script for generating input data and ground truth data
│   │   └── verify_result.py           // Script for verifying whether output data matches ground truth data
│   ├── CMakeLists.txt                 // Build project file
│   ├── data_utils.h                   // Data read/write functions
│   ├── fixpipe_l0c2l1.asc             // Ascend C example implementation & invocation example
│   └── README.md                      // Example documentation
```

## FixpipeParamsV220 vs FixpipeParamsArch3510 Structure Comparison

Different products support different parameter structures:
- **Ascend 950PR/Ascend 950DT**: Supports both `FixpipeParamsV220` and `FixpipeParamsArch3510` parameter structures, `FixpipeParamsArch3510` is recommended
- **Atlas A3 Training/Inference Series, Atlas A2 Training/Inference Series**: Only supports `FixpipeParamsV220`

This example selects different architectures through the compilation parameter `CMAKE_ASC_ARCHITECTURES`, automatically selecting the corresponding parameter structure based on architecture:
- `dav-2201` architecture: Uses `FixpipeParamsV220`
- `dav-3510` architecture: Uses `FixpipeParamsArch3510`

| Member Name | FixpipeParamsV220 | FixpipeParamsArch3510 | Description |
| --- | --- | --- | --- |
| nSize | Yes | Yes | Output matrix size in N direction |
| mSize | Yes | Yes | Output matrix size in M direction |
| srcStride | Yes | Yes | Start address offset of adjacent Z layouts in source Nz matrix |
| dstStride | Yes | Yes | Start address offset of adjacent Z layouts in destination matrix (Nz format, note unit differences between structures) or elements per row (ND/DN format) |
| quantPre | Yes | Yes | Quantization mode control |
| deqScalar | Yes | Yes | Scalar quantization parameter |
| reluEn | Yes | Yes | ReLU switch |
| unitFlag | Yes | Yes | Mmad and Fixpipe fine-grained parallelism control |
| isChannelSplit | Yes | Yes | Channel split switch |
| ndNum / srcNdStride / dstNdStride | Yes | Yes (in TransformParams) | Parameters controlling multi-matrix transfer in Nz2ND scenarios, independent members in V220, integrated into TransformParams structure in Arch3510 |
| dnNum / srcNzMatrixStride / dstDnMatrixStride / srcNzC0Stride | No | Yes (in TransformParams) | Parameters controlling multi-matrix transfer in Nz2DN scenarios, only Arch3510 supported |
| TransformParams | No | Yes | Template parameter-based type selector, automatically selects parameter type based on CO2Layout |
| dualDstCtrl | No | Yes | Dual-target mode control, supports M dimension or N dimension splitting |
| subBlockId | No | Yes | Target UB number indicator in single-target mode |

## Scenario Description

This example selects different output scenarios through the compilation parameter `SCENARIO_NUM`. The meanings of different `SCENARIO_NUM` values are shown in the table below.
All scenarios are based on the same matrix multiplication specification: [M, N, K] = [128, 128, 128], with kernel function name `fixpipe_l0c2l1`.

| scenarioNum | L0C Data Type | L1 Data Type | Output Format | Quantization Enabled | ReLU Enabled |
| --- | --- | --- | --- | --- | --- |
| 1 | float | half | Nz | No (cast) | No |
| 2 | float | int8_t | Nz | Yes (scalar) | No |
| 3 | float | int8_t | Nz | Yes (vector) | No |
| 4 | float | half | Nz | No (cast) | Yes |

**Scenario 1: Output format Nz, output to L1 data type half**
- Input: A [128, 128] half type, ND format; B [128, 128] half type, ND format
- Output: C [128, 128] half type, Nz format
- Implementation: Use `Fixpipe<outputType, l0cType, AscendC::CFG_Nz>` to transfer data from L0C to L1, output in Nz format
- Description: L0C data in Nz format directly output to L1 in Nz format, data remains unchanged

**Scenario 2: Output format Nz, output to L1 data type int8_t, Scalar quantization enabled**
- Input: A [128, 128] half type, ND format; B [128, 128] half type, ND format
- Output: C [128, 128] int8_t type, Nz format
- Implementation: Set `fixpipeParams.quantPre = QuantMode_t::QF322B8_PRE`, use Scalar quantization mode
- Description: Quantize float type data to int8_t type, entire C matrix uses one quantization parameter

**Scenario 3: Output format Nz, output to L1 data type int8_t, Vector quantization enabled**
- Input: A [128, 128] half type, ND format; B [128, 128] half type, ND format
- Output: C [128, 128] int8_t type, Nz format
- Implementation: Set `fixpipeParams.quantPre = QuantMode_t::VQF322B8_PRE`, use Vector quantization mode, pass per-column quantization parameters through quantAlphaTensor
- Description: Quantize float type data to int8_t type, each column of C matrix corresponds to one quantization parameter, quantization parameters need to be copied from GM to L1

**Scenario 4: Output format Nz, output to L1 data type half, ReLU enabled**
- Input: A [128, 128] half type, ND format; B [128, 128] half type, ND format
- Output: C [128, 128] half type, Nz format
- Implementation: Set `fixpipeParams.reluEn = true` to enable ReLU feature
- Description: Perform ReLU operation during data transfer from L0C to L1, setting negative values to 0

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
  cmake -DSCENARIO_NUM=$SCENARIO_NUM -DCMAKE_ASC_ARCHITECTURES=dav-2201 ..;make -j;    # Build the project (default dav-2201 NPU mode)
  python3 ../scripts/gen_data.py -scenarioNum=$SCENARIO_NUM  # Generate test input data
  ./demo                           # Run the compiled executable to execute the example
  python3 ../scripts/verify_result.py output/output.bin ./output/golden.bin $SCENARIO_NUM # Verify whether the output result is correct
  ```

  To use CPU debug or NPU simulation mode, add the `-DCMAKE_ASC_RUN_MODE=cpu` or `-DCMAKE_ASC_RUN_MODE=sim` parameter.

  Examples:

  ```bash
  cmake -DCMAKE_ASC_RUN_MODE=cpu -DSCENARIO_NUM=$SCENARIO_NUM -DCMAKE_ASC_ARCHITECTURES=dav-2201 ..;make -j;  # CPU debug mode
  cmake -DCMAKE_ASC_RUN_MODE=sim -DSCENARIO_NUM=$SCENARIO_NUM -DCMAKE_ASC_ARCHITECTURES=dav-2201 ..;make -j;  # NPU simulation mode
  ```
  > **Notice:** Clear the cmake cache before switching build modes. Run `rm CMakeCache.txt` in the build directory and then re-run cmake.

- Build option description

  | Option | Values | Description |
  |--------|--------|-------------|
  | `CMAKE_ASC_RUN_MODE` | `npu` (default), `cpu`, `sim` | Run mode: NPU execution, CPU debug, NPU simulation |
  | `CMAKE_ASC_ARCHITECTURES` | `dav-2201` (default), `dav-3510` | NPU architecture: dav-2201 corresponds to Atlas A2 Training Series Products/Atlas A2 Inference Series Products and Atlas A3 Training Series Products/Atlas A3 Inference Series Products, dav-3510 corresponds to Ascend 950PR/Ascend 950DT |
  | `SCENARIO_NUM` | 1-4 | Scenario number |

  The following execution result indicates that the accuracy comparison is successful.

  ```bash
  test pass!
  ```
