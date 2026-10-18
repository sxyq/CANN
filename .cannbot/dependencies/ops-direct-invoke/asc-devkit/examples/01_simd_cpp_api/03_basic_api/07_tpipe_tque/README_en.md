# TPipe/TQue Resource Management API Sample Introduction

## Overview

This directory contains examples for multiple APIs related to TPipe/TQue resource management. Each example is based on the Ascend C <<<>>> direct call method and demonstrates TPipe/TQue-related interfaces.

## Example List

| Directory Name | Description | Supported Products |
| ----------------------------------------------------------- | --------------------------------------------------- | --- |
| [get_tpipe_ptr](./get_tpipe_ptr) |  This example obtains the global TPipe pointer based on GetTPipePtr, allowing the kernel function to perform TPipe-related operations without explicitly passing the TPipe pointer. | Ascend 950PR/Ascend 950DT
Atlas A3 Training Series Products/Atlas A3 Inference Series Products
Atlas A2 Training Series Products/Atlas A2 Inference Series Products |
| [tpipe_reuse](./tpipe_reuse) |  This example implements TPipe repeated allocation and usage based on TPipe::Init and TPipe::Destroy. | Ascend 950PR/Ascend 950DT
Atlas A3 Training Series Products/Atlas A3 Inference Series Products
Atlas A2 Training Series Products/Atlas A2 Inference Series Products |
