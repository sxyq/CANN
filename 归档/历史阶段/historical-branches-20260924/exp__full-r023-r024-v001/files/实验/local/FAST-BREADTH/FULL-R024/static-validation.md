# FULL-R024 V001 static validation

- `run_kernel` is present and launches a complete device-side AddRmsNormBias kernel.
- The CPU matrix is an independent reference executable and never supplies device output.
- Kernel arithmetic remains device-side: residual add, square, reduction, epsilon, sqrt, normalization, gamma and bias.
- The kernel validates rank, dtype, shape compatibility, 64-bit row arithmetic and valid tail byte counts.
- The matrix covers FP32/FP16/BF16, rank 2/3/4, non-32B widths, multiple outer sizes, two epsilon values and special rows.
