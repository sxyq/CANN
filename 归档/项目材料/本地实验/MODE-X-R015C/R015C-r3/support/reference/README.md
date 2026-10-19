# R015C-r3 correctness reference

This directory is a route-local correctness reference only. It keeps the
same `RowCopyTiling` ABI, host probe, input formula, and FP32 shape set as
R015C-r3. Each GM-to-UB and UB-to-GM `DataCopyPad` request carries one row
segment and stays at or below 256 32-byte blocks. No timing or submission
result is associated with this reference.
