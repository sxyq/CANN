# Activation Function Operator Examples

## Overview

This example set demonstrates typical usage of activation function high-level APIs, with each example containing a complete end-to-end implementation.

## Example List

| Directory Name | Description | Supported Products |
| :------------------------------------------------------------ | :---------------------------------------------------- | --- |
| [geglu](./geglu) |  This example demonstrates the implementation based on the GeGLU high-level API, supporting GLU variant scenarios where GELU activation is used as the activation function | Ascend 950PR/Ascend 950DT
Atlas A3 Training Series Products/Atlas A3 Inference Series Products
Atlas A2 Training Series Products/Atlas A2 Inference Series Products |
| [gelu](./gelu) |  This example demonstrates the operator implementation based on the Gelu high-level API. The example performs GELU activation computation on the input Tensor element-wise | Ascend 950PR/Ascend 950DT
Atlas A3 Training Series Products/Atlas A3 Inference Series Products
Atlas A2 Training Series Products/Atlas A2 Inference Series Products |
| [logsoftmax](./logsoftmax) |  This example demonstrates the operator implementation based on the LogSoftMax high-level API. The example performs LogSoftmax computation on the input tensor | Ascend 950PR/Ascend 950DT
Atlas A3 Training Series Products/Atlas A3 Inference Series Products
Atlas A2 Training Series Products/Atlas A2 Inference Series Products |
| [sigmoid](./sigmoid) |  This example demonstrates the operator implementation based on the Sigmoid high-level API. The example performs logistic regression Sigmoid element-wise | Ascend 950PR/Ascend 950DT
Atlas A3 Training Series Products/Atlas A3 Inference Series Products
Atlas A2 Training Series Products/Atlas A2 Inference Series Products |
| [softmax](./softmax) |  This example introduces calling the SoftMax high-level API to implement a single softmax operator. For an input tensor[m0, m1, ...mt, n] (where t is greater than or equal to 0), the product of non-last axis lengths is treated as m, so the input tensor shape is viewed as [m, n] | Ascend 950PR/Ascend 950DT
Atlas A3 Training Series Products/Atlas A3 Inference Series Products
Atlas A2 Training Series Products/Atlas A2 Inference Series Products |
| [softmaxflashv2](./softmaxflashv2) |  This example introduces calling the SoftmaxFlashV2 high-level API to implement a single softmaxflashv2 operator, which is an enhanced version of SoftmaxFlash corresponding to the FlashAttention-2 algorithm | Ascend 950PR/Ascend 950DT
Atlas A3 Training Series Products/Atlas A3 Inference Series Products
Atlas A2 Training Series Products/Atlas A2 Inference Series Products |
| [softmaxflashv3](./softmaxflashv3) |  This example introduces calling the SoftmaxFlashV3 high-level API to implement a single softmaxflashv3 operator, which is an enhanced version of SoftmaxFlash corresponding to the Softmax PASA algorithm | Ascend 950PR/Ascend 950DT
Atlas A3 Training Series Products/Atlas A3 Inference Series Products
Atlas A2 Training Series Products/Atlas A2 Inference Series Products |
| [softmaxgrad](./softmaxgrad) |  This example introduces calling the SoftmaxGrad high-level API to implement a single softmaxgrad operator. For an input tensor[m0, m1, ...mt, n] (where t is greater than or equal to 0), the product of non-last axis lengths is treated as m, so the input tensor shape is viewed as [m, n] | Ascend 950PR/Ascend 950DT
Atlas A3 Training Series Products/Atlas A3 Inference Series Products
Atlas A2 Training Series Products/Atlas A2 Inference Series Products |
| [swish](./swish) |  This example demonstrates the operator implementation based on the Swish/Silu high-level APIs, with compilation macros to switch between two modes. Swish performs activation computation element-wise, and Silu is a special case of Swish with beta=1 | Ascend 950PR/Ascend 950DT
Atlas A3 Training Series Products/Atlas A3 Inference Series Products
Atlas A2 Training Series Products/Atlas A2 Inference Series Products |
| [swiglu](./swiglu) |  This example demonstrates the operator implementation based on the SwiGLU high-level API. The example uses Swish as the activation function for the GLU variant | Ascend 950PR/Ascend 950DT
Atlas A3 Training Series Products/Atlas A3 Inference Series Products
Atlas A2 Training Series Products/Atlas A2 Inference Series Products |
