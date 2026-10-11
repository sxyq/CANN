# N04 V006 Handoff

V006 uses the V005 result tree as its parent and changes one existing input-path cache mode: `xGm_` changes from `CACHE_MODE_DISABLE` to `CACHE_MODE_LAST`. The output `CACHE_MODE_PERSISTENT` remains unchanged. Candidate commit is `5834f62e2f7ae19979cf1457a0251627819c3d3c`; paired harness commit is `4e5eb9e6160cd923f4a6890700ba1647078071b4`; source SHA256 is `3b79b6d432d3e41ffb794a7656f4eddd2e3381efdc9c9717a83fb5f5ef537247`.

Adapter/device and paired builds passed. Correctness passed all four public cases with zero failures. Paired Local completed 21 alternating pairs per case on d1. Raw output, device snapshots, build logs, upload verification, and compact binaries are in this directory.

`LOCAL_SCORE=NONE`: the paired deltas have large outliers and MAD values larger than the median changes, while d1 carried unrelated load. Do not claim a benefit from this result. Main's FULL_SOURCE_SIMILARITY results around `0.982-0.999` remain recorded as an over-limit fact; no cosmetic changes were used to affect them.

The first correctness invocation exited 127 because CANN loader paths were omitted. It was preserved, then the same Runner completed with `/usr/lib/aarch64-linux-gnu` first and the existing CANN paths present. No other process was changed.

Next work, if continued, should stay on the existing GM/L2 single-factor axis and must not add a second Hint or modify DMA, tiling, arithmetic, or parameter residency.
