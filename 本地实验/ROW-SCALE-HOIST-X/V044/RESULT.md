# V044 Result

- Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`).
- Change: In FP16 `ProcessNarrowMidOverlap`, when `localRows==1`, apply the existing half `invRms` multiply to the freshly loaded gamma tile before multiplying the output tile by gamma. The `localRows>1` resident-parameter path remains unchanged.
- Compile: PASS for `device` and `submission` on `hwnput3`, Ascend910B3 / `dav-2201`, toolkit `8.5.0.alpha002`. The initial environment helper path was absent; explicit CMake compiler/toolkit paths worked. Correctness and Local used the valid toolkit environment script.
- Correctness: Parent and Candidate PASS on device 0 for FP16 `[40,3072]`, 40 blocks, one row/core, `residentParams=false`. Both matched ratio `1.0` and max absolute error `0.00390625` under `atol=rtol=1/512`, matched ratio `>=0.99`, max error `<=0.1`.
- Local: 96 device-event samples per arm, 20 warmups per runner, three interleaved Parent/Candidate blocks. Pooled medians were Parent `17.29 us` and Candidate `20.9299995 us`; descriptive score `82.6086976256`, delta `+21.0526286871%` (Candidate slower). Paired deltas were `+31.853073%`, `+104.872396%`, `+6.582273%` (positive is slower).
- Local verdict: `MEASUREMENT_BLOCKED`, not a reliable Local rejection/promotion. Parent same-binary medians shifted from `19.58` to `12.5099995 us` across three windows (`-36.108276%`); pooled CV was `0.406737` Parent / `0.316809` Candidate. All raw samples are retained with no exclusions. `CURRENT_LOCAL_BEST=V026`.
- Load: device 0 had no running processes and `0%` AICore use before/after; HBM stayed near `5%` (3432/3434 MB). Host load averages were `44.49,47.17,44.88` before and `36.08,44.50,44.08` after; other NPUs had shared activity. No other process was modified.
- Device 0: reserved under the direct user instruction for V044 correctness and Local through numeric capture; Parent/Candidate runs were serialized. A stale 2026-09-27 device-0 `LEASED` row for MAIN-1/MIX-A exists in this worktree's shared ledger and was not altered. Live process snapshots showed device 0 idle. The final post-capture snapshot at `2026-10-08T00:48:57Z` showed healthy NPU 0, AICore `0%`, HBM `3432/65536 MB`, and no running process; reservation release at `00:48:59Z` is recorded in `device-release-20261008T004600Z.log`.
- Official score absent; Online not run.

Evidence: compile, correctness build/run, Local build/raw data, reservation/release snapshots, declaration, source hash, and OFAT diff are retained in this directory.
