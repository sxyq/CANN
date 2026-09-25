# MODE-X-R015C R015C-r4 Handoff

## Current state

- Approved hypothesis retained: one row per block; segment size, DataCopyPad path and order, and 64 KiB VECCALC scratch remain unchanged.
- Source SHA-256: `9367db4ebb4edf6b7bf6cde97f4846f987e6ab230c2aeee44d942d5e881a1c74`.
- Direct parent: R015C-r3, source SHA-256 `e1786bec2673519f41887fdffa0e11751f515a81037d854c88ae7edc0e4af903`.
- Compile and link: PASS on `cann-server3` (`hwnput3`), CANN `8.5.0.alpha002`, target `dav-2201`.
- Correctness: PASS on device 4 for `(2,256)`, `(5,4096)`, and `(3,8192)`; each output matched the input exactly.
- Performance timing: NOT RUN. No r2/r3 comparison was run.

## Executable identity

- Path: `/home/data4t2/lelinfeng/MODE-X-R015C_runs/R015C-r4/build/row_copy_reference`
- SHA-256: `21934a8cf15508b365e252810c8f858b4fb105c5cc5000f460d61c511a37e188`
- Size: 384752 bytes; ELF 64-bit PIE, ARM aarch64.

## Evidence

- Build logs: `support/server3/configure.log`, `support/server3/build-attempt-1.log`, and `support/server3/build-attempt-2.log`.
- Correctness log: `support/server3/correctness.log`.
- Machine-readable result: `local-result.json`.
- Source metadata: `source-meta.json`.

The first build attempt lacked the standard C++ include path used by the existing Route build setup. The second attempt exported `CPLUS_INCLUDE_PATH` using that setup and completed compilation and linking. No Candidate source or CMake change was needed.

## Main review handoff

Review the recorded source SHA, approved one-row-per-block diff, executable identity, build logs, and exact correctness results. Performance remains unmeasured; this handoff makes no performance or promotion conclusion. Stop here for Main review.
