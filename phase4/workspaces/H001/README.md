# H001 AddRmsNormBias

This candidate targets small `D` and high `R` on Ascend910B3/DAV_2201.

- `D <= 1024`: eight rows are staged per tile, gamma and bias stay resident for the whole invocation, and 32-byte-aligned tiles use one padded DMA for multiple rows.
- `D > 1024`: a one-row chunked fallback handles every legal width without assuming row alignment.
- FP16, BF16, and FP32 have separate kernel entry points and accumulate the RMS in FP32.

The only source entry points are in `src/add_rms_norm_bias_kernel.cpp`. `build_server3.sh` is intended to run from a server3 checkout with CANN 8.5.0.alpha002.
