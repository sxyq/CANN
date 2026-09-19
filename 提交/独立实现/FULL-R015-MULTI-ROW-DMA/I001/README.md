# FULL-R015-MULTI-ROW-DMA I001

This is the blinded multi-row DMA candidate. The local source files use `.txt`; the
server compile script materializes the fixed `.asc` entry names required by the
Ascend C build.

The kernel computes, for FP32 tensors shaped as `rows x D`:

`y = (x + residual) * rsqrt(mean((x + residual)^2) + epsilon) * gamma + bias`

Rows are flattened from any 2D/3D/4D input by the caller.
