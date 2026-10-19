# MODE-X-R015C

Self-contained contiguous FP32 row-copy candidate. Public inputs are a row-major
`[R, D]` tensor and an equally shaped output; `R > 1`, `256 <= D <= 8192`, and
`D` must be divisible by 8 for 32-byte-aligned `DataCopy`. The kernel groups two
adjacent rows into one 2-D `DataCopy` in each direction. Host ABI does not expose
strides: both tensors are standard contiguous buffers.

Run `bash build_and_probe.sh` after sourcing the server CANN environment. This
builds and links the executable and runs host probes. Run
`bash build_and_probe.sh --device-probes` only when an NPU is available; it
checks `(R=2,D=256)` and `(R=5,D=4096)` exactly against the contiguous input.
