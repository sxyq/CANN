# EPI-X-FRESH

This candidate implements AddRmsNormBias for DAV_2201.

The device path treats the flattened leading dimensions as rows. Each block
handles a contiguous row range and uses an eight-row batch. A column chunk is
loaded once for the parameter vectors and is reused by every row in the batch.
The first pass accumulates the squared sum in FP32. The second pass recomputes
the fused add and writes the normalized epilogue. Only the final column chunk
uses padding, and the output copy writes only valid elements.

Build and local device probes run through `run_local.sh` on `cann-server3`.
