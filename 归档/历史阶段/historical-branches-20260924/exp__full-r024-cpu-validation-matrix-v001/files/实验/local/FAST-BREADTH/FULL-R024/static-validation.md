# FULL-R024 V001 static validation

- `run_kernel` is present and launches the complete device-side AddRmsNormBias chain.
- The CPU runner is independent and never supplies device output.
- The candidate has distinct matrix-driven paths: FP32 tile 2048, FP16/BF16 tile 4096, aligned `DataCopy`, and padded `DataCopyPad`.
- The matrix includes 1080 cases and records both aligned and padded data/parameter transfers.
- The candidate source is not the R023 source with identifiers renamed; its tile and transfer decisions differ in executable code.
