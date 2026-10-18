# Conv2D Forward Tensor API Sample

## Overview

This sample demonstrates how to implement Conv2D Forward computation using the Tensor API. It shows the complete pipeline: moving the FeatureMap from Global Memory to L1 Buffer, performing spatial unfolding via `copy_l1_to_l0a` (Img2Col), accumulating matrix multiplication with `mmad`, and moving results back to Global Memory via `copy_l0c_to_gm`.

This sample supports three Global Memory input/output data formats (NC1HWC0, NCHW, NHWC), switched via the compile-time parameter `SCENARIO_NUM`. It uses a 3x3 convolution kernel with padding, stride=2, dilation=2, and multiple C1 channel combinations to demonstrate common convolution scenarios and Tensor API usage.

## Supported Products and CANN Software Versions

| Product | CANN Software Version |
|---------|----------------------|
| Ascend 950PR/Ascend 950DT | >= CANN 9.1.0 |

## Directory Structure

```text
├── conv2d_forward_tensor_api
│   ├── scripts
│   │   ├── gen_data.py                // Script for generating input and golden data
│   │   └── verify_result.py           // Script for verifying output against golden data
│   ├── CMakeLists.txt                 // Build project file
│   ├── data_utils.h                   // Data read/write functions
│   ├── conv2d_forward_tensor_api.asc  // Ascend C Tensor API sample implementation and invocation
│   └── README.md                      // Sample documentation
```

## Sample Specifications

This sample uses the following fixed Conv2D specification. The kernel function name is `conv2d_forward_tensor_api_custom`. Each AI Core processes one batch, and 4 AI Cores are used in total.

| Parameter | Value | Description |
| --- | --- | --- |
| FeatureMap | [N, H, W, C] = [4, 9, 9, 32] | Global Memory input is half type, layout selected per scenario: NC1HWC0/NCHW/NHWC |
| Weight | [Cout, C, Kh, Kw] = [16, 32, 3, 3] | Global Memory input is half type, laid out as [K, Cout] in 2D |
| Convolution params | stride = [2, 2], dilation = [2, 2], padding = [2, 2, 2, 2], padValue = 1 | padding order is [top, bottom, left, right] |
| Output | [N, HOut, WOut, Cout] = [4, 5, 5, 16] | Global Memory output is half type, same format as input |

Here, `C1` and `C0` are the channel split dimensions of the FeatureMap when using the NC1HWC0 layout on the L1 Buffer. `C0` represents the number of elements per channel block, which is 16 in this sample; `C1` represents the number of channel blocks. Since `C = 32`, we have `C1 = C / C0 = 2` and `C0 = 16`.

The effective kernel size is calculated as:

$$
K_{\text{effective}} = dilation \times (K_h - 1) + 1 = 2 \times (3 - 1) + 1 = 5
$$

The output H/W is calculated as:

$$
\begin{aligned}
H_{out} &= \frac{H + pad_{top} + pad_{bottom} - K_{\text{effective}}}{stride_h} + 1 = \frac{9 + 2 + 2 - 5}{2} + 1 = 5 \\
W_{out} &= \frac{W + pad_{left} + pad_{right} - K_{\text{effective}}}{stride_w} + 1 = \frac{9 + 2 + 2 - 5}{2} + 1 = 5
\end{aligned}
$$

The matrix multiplication specification is `M = HOut * WOut = 25`, `K = C * Kh * Kw = 288`, `N = Cout = 16`.

## Scenario Details

This sample selects different Global Memory data formats via the compile parameter `SCENARIO_NUM`. The meanings of different `SCENARIO_NUM` values are shown in the table below.

| SCENARIO_NUM | FeatureMap Input Layout | Output Layout | Global Memory → L1 Buffer Conversion | L0C Buffer → Global Memory Conversion |
| --- | --- | --- | --- | --- |
| 0 | NC1HWC0 [1, C1, H, W, C0] | NC1HWC0 [1, Cout_C1, HOut, WOut, C0] | NC1HWC0 direct copy | NC1HWC0 direct copy |
| 1 | NCHW [1, C, H, W] | NCHW [1, Cout, HOut, WOut] | NCHW → NC1HWC0 (dn2nz) | NC1HWC0 → NCHW (nz2dn) |
| 2 | NHWC [1, H, W, C] | NHWC [1, HOut, WOut, Cout] | NHWC → NC1HWC0 (nd2nz) | NC1HWC0 → NHWC (nz2nd) |

**Scenario 0: NC1HWC0 Input/Output**
- Input: FeatureMap [1, 2, 9, 9, 16] half type, NC1HWC0 format; Weight [K, Cout] half type
- Output: Output [1, 1, 5, 5, 16] half type, NC1HWC0 format
- Implementation: Global Memory → L1 Buffer uses `copy_gm_to_l1` to directly copy NC1HWC0 data; L0C Buffer → Global Memory uses `copy_l0c_to_gm` direct copy

**Scenario 1: NCHW Input/Output**
- Input: FeatureMap [1, 32, 9, 9] half type, NCHW format; Weight [K, Cout] half type
- Output: Output [1, 16, 5, 5] half type, NCHW format
- Implementation: Global Memory → L1 Buffer uses `copy_gm_to_l1` to convert NCHW to NC1HWC0 on the fly (dn2nz); L0C Buffer → Global Memory uses `copy_l0c_to_gm` to convert NC1HWC0 to NCHW (nz2dn)

**Scenario 2: NHWC Input/Output**
- Input: FeatureMap [1, 9, 9, 32] half type, NHWC format; Weight [K, Cout] half type
- Output: Output [1, 5, 5, 16] half type, NHWC format
- Implementation: Global Memory → L1 Buffer uses `copy_gm_to_l1` to convert NHWC to NC1HWC0 on the fly (nd2nz); L0C Buffer → Global Memory uses `copy_l0c_to_gm` to convert NC1HWC0 to NHWC (nz2nd)

Core workflow:

1. Move the single-batch FeatureMap from Global Memory to L1 Buffer by calling the copy interface via the `copy_gm_to_l1` `Operation` capability, unifying to the NC1HWC0 layout. `copy_gm_to_l1` auto-routes the copy path based on the Global Memory source format (NCHW via dn2nz, NHWC via nd2nz, NC1HWC0 direct). Simultaneously, move the Weight from Global Memory to L1 Buffer, performing ND-to-ZN format conversion.
2. Perform img2col spatial unfolding of the FeatureMap on L1 Buffer to L0A Buffer, calling the copy interface via the `copy_l1_to_l0a` `Operation` Img2Col capability, completing the 3x3 kernel unfolding and padding under stride=2, dilation=2.
3. Move the Weight from L1 Buffer to L0B Buffer, calling the copy interface via the `copy_l1_to_l0b` `Operation` capability.
4. Use `mmad` to perform matrix multiplication: L0A Buffer(NZ) × L0B Buffer(ZN) → L0C Buffer(NZ).
5. Use `copy_l0c_to_gm` to move the L0C Buffer result back to Global Memory in the target format (NCHW via nz2dn, NHWC via nz2nd, NC1HWC0 direct).

## Build and Run

Execute the following steps in the sample root directory to build and run the sample.

- Configure environment variables

  Refer to the CANN development kit [installation guide](../../../../docs/en/quick_start.md#prepare&install) to configure environment variables.
  ```bash
  source ${install_path}/cann/set_env.sh
  ```

  > **Note:** `${install_path}` is the CANN installation directory. If not specified, it defaults to `/usr/local/Ascend`.

- Run the sample

  Execute the following commands in the sample directory.
  ```bash
  SCENARIO_NUM=0
  mkdir -p build && cd build;      # Create and enter the build directory
  cmake -DSCENARIO_NUM=$SCENARIO_NUM -DCMAKE_ASC_ARCHITECTURES=dav-3510 ..;make -j;    # Build the project (default dav-3510 NPU mode)
  python3 ../scripts/gen_data.py --scenario=$SCENARIO_NUM   # Generate test input data
  ./demo                           # Run the compiled executable
  python3 ../scripts/verify_result.py output/output.bin output/golden.bin   # Verify output correctness
  ```

  To switch input/output format: change `-DSCENARIO_NUM=0` to `1` (NCHW) or `2` (NHWC), then rebuild and run.

  To use NPU simulation mode, add the `-DCMAKE_ASC_RUN_MODE=sim` parameter.

  Example:

  ```bash
  cmake -DSCENARIO_NUM=$SCENARIO_NUM -DCMAKE_ASC_RUN_MODE=sim -DCMAKE_ASC_ARCHITECTURES=dav-3510 ..;make -j;  # NPU simulation mode
  ```

  > **Note:** Before switching build modes, clean the cmake cache by running `rm CMakeCache.txt` in the build directory, then re-run cmake.

- Build options

  | Option | Values | Description |
  |--------|--------|-------------|
  | `CMAKE_ASC_RUN_MODE` | `npu` (default), `sim` | Run mode: NPU execution, NPU simulation |
  | `CMAKE_ASC_ARCHITECTURES` | `dav-3510` | NPU architecture: dav-3510 corresponds to Ascend 950PR/Ascend 950DT |
  | `SCENARIO_NUM` | `0` (default), `1`, `2` | Scenario number: 0=NC1HWC0, 1=NCHW, 2=NHWC |

- Execution result

  The following output indicates successful precision comparison.
  ```bash
  test pass!
  ```
