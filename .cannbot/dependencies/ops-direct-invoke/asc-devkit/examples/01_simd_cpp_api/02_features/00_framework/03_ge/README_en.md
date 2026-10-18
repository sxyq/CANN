# GE Example Introduction

## Overview

This directory introduces how to connect Ascend C custom operators to GE native graph execution.

## Example List

| Directory Name | Description |
| --- | --- |
| [triple_chevron_notation](./triple_chevron_notation) | Example showing how to call an Ascend C kernel with `<<<>>>` directly in GE `EagerExecuteOp::Execute` and execute `AddCustom` through GE native graph construction |
