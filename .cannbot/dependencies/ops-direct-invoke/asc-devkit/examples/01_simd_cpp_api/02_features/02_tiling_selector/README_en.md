# Tiling Selector Sample Introduction

## Overview

This directory introduces Ascend C SIMD Tiling Selector feature samples. It demonstrates typical workflows for calculating Tiling parameters based on input data volume and the number of AI Cores, and for selecting inter-core and intra-core split strategies.

## Operator Development Samples

| Directory Name | Description |
| -------------- | ----------- |
| [vector_tiling_strategy](./vector_tiling_strategy) | Uses one-dimensional Tensor addition to demonstrate multi-core Tiling split strategy calculation and selection for vector computing. |
