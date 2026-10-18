# Compile Sample Introduction

## Overview

This document presents compile samples based on Ascend C SIMD, demonstrating various SIMD compilation modes.

## Operator Development Samples

| Directory Name | Description |
|-----------------------------------------------------|------------------------|
| [00_basic_compile](./00_basic_compile) | Demonstrates the basic heterogeneous compilation workflow of Host-side .cpp and Device-side .asc mixed files. |
| [01_separate_compile](./01_separate_compile) | Demonstrates splitting Device-side code into multiple .asc files with separate compilation, enabling cross-file device function calls. |
| [02_dynamic_library_compile](./02_dynamic_library_compile) | Demonstrates compiling Device-side .asc files into dynamic shared libraries (.so). |
| [03_static_library_compile](./03_static_library_compile) | Demonstrates compiling Device-side .asc files into static libraries (.a). |
