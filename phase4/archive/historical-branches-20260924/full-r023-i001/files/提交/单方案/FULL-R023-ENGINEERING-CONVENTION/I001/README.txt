FULL-R023 I001 AddRmsNormBias

The device entry points are add_rms_norm_bias_fp16, add_rms_norm_bias_bf16,
and add_rms_norm_bias_fp32. The flattened outer dimension and the last
dimension are passed as uint64_t kernel arguments. The implementation keeps
the reduction and affine arithmetic in float and casts only at the GM boundary.

Compile on the designated CANN 9.0.0 host with:

    bash build.sh

This project intentionally contains no runtime or correctness test harness.
