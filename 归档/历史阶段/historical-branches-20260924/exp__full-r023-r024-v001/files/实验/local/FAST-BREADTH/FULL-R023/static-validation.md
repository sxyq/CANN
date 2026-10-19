# FULL-R023 V001 static validation

- `run_kernel` is present and launches a `__global__ __vector__` kernel.
- Kernel arithmetic remains device-side: residual add, square, reduction, epsilon, sqrt, normalization, gamma and bias.
- Host shape validation rejects null pointers, unsupported dtype, rank mismatch, gamma/bias mismatch and multiplication overflow.
- Row, column and byte offsets use `uint64_t`; device launch count is bounded before conversion to `uint32_t`.
- Input tails use `DataCopyPad`; stores use the valid byte count and do not write padded output.
- The route source contains no `pipe_` identifier; the TPipe member is `workPipe`.
- CPU/reference output is not passed into the kernel.
