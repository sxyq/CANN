# Memory Optimizations Example Introduction

## Overview

Using memory access optimization with direct `<<<>>>` invocation, this section introduces memory access tuning approaches based on  SIMT programming, including global memory access coalescing, on-chip cache reuse (e.g., UB), Bank conflict avoidance, and improving memory bandwidth utilization via short vector such as half2.

## Example List

| Directory Name                                              | Description                                                                                        |
| ------------------------------------------------------ | ----------------------------------------------------------------------------------------------- |
| [matrix_transpose_practice](./matrix_transpose_practice)   | This example uses matrix transpose to demonstrate the memory access tuning path in SIMT programming, starting from direct-index transpose and progressively introducing UB staging, global memory coalescing, thread block mapping and thread count selection, UB bank conflict avoidance through padding, and double buffering to remove trailing synchronization. |
| [short_vector_add](./short_vector_add)            | This example implements the Add operator based on Ascend C SIMT programming, demonstrating performance tuning methods based on the short vector half2.                           |
| [cache_hint](./cache_hint)     | This example implements the Sin operator based on Ascend C SIMT programming, demonstrating the cache optimization capabilities of SIMT programming.                    |
| [aligned_types](./aligned_types)          | This example, based on Ascend C SIMT programming, demonstrates the impact of struct type alignment on Global Memory access throughput. |
