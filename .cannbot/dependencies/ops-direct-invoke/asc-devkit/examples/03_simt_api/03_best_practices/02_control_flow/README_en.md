# Control Flow Optimizations Sample Introduction

## Overview

Control flow optimization samples, implemented through direct <<<>>> invocation, introduce the impact of Warp Divergence on performance and optimization approaches in SIMT programming. Currently, optimization cases for reducing branch divergence through Warp cooperative processing are provided.

## Sample List

| Directory Name                                              | Description                                                                                        |
| ------------------------------------------------------ | ----------------------------------------------------------------------------------------------- |
| [warp_divergence](./warp_divergence)   | Using sparse matrix-vector multiplication (SpMV) as an example, this sample compares "one thread processing one row of data" and "one Warp cooperatively processing one row of data" approaches, demonstrating the impact of Warp Divergence on performance and optimization methods. |
