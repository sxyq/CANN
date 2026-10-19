# MODE-X-R015C

MODE-X-R015C R015C-r4 copies contiguous FP32 rows. Inputs are a row-major
`[R, D]` tensor and an equally shaped output; `R > 1`, `256 <= D <= 8192`, and
`D` is divisible by 8. Each block handles one row. The row is copied in segments
of at most 2048 FP32 values using `DataCopyPad` in both directions, with a full
barrier after each copy before the 64 KiB VECCALC staging buffer is reused.
Host ABI does not expose strides: both tensors are standard contiguous buffers.

The exact parent and candidate sources are retained under
`phase4/local/MODE-X-R015C/R015C-r3/` and
`phase4/local/MODE-X-R015C/R015C-r4/`. The paired event runner under
`support/paired_runner/` links one host executable with separate r3 and r4
Ascend C kernel libraries. It retains each source's original kernel entry and
tiling header while running both through one process and ACL stream. Its
sequence qualifies the r3 parent on the requested shape first, using at least
10 warmups, two in-process blocks, and at least 21 device-event samples per
block. It enters interleaved PC/CP pairs only when the parent shape passes the
recorded MAD and block-drift limits. A current exclusive MAIN-1 device lease is
required to run the device mode. Host-only identity and argument tests do not
initialize ACL or access an NPU.
