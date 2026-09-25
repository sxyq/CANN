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

## Paired runner preparation (2026-09-25)

- Runner source: `phase4/workspaces/MODE-X-R015C/support/paired_runner/`.
- Server: `cann-server3` (`hwnput3`), CANN `8.5.0.alpha002`, target `dav-2201`.
- Build directory: `/home/data4t2/lelinfeng/MODE-X-R015C_runs/R015C-r4/paired-runner/build-host-only-6/`.
- Runner executable: `build-host-only-6/r015c_pair_runner`, SHA-256 `0b576bea88cc5c6664728f3e5a1f749e1560f189fab9b6b714fd2cbd578c5837`.
- Parent module: `build-host-only-6/libr015c_parent_kernel.so`, SHA-256 `8f57bc1b028f815e4f1cea53f5b5d22781bddb6077b7cc3a47acf70123ab4486`.
- Candidate module: `build-host-only-6/libr015c_candidate_kernel.so`, SHA-256 `9339fd686d9498fc55d899aa0ef6dac6c49a7092b93b0b36e7a38eaed6f66b33`.
- Parent source / tiling SHA-256: `e1786bec2673519f41887fdffa0e11751f515a81037d854c88ae7edc0e4af903` / `5e4ad750f5c63da357324c0a829674aff81b7c0b655e8ff08072b1c289c66c41`.
- Candidate source / tiling SHA-256: `9367db4ebb4edf6b7bf6cde97f4846f987e6ab230c2aeee44d942d5e881a1c74` / `0939ba8498426fcd77645d826fcc45a8eb65c78996ad0ce1da1a00d1ac01a250`.
- Build and link: PASS. Host identity and argument tests: PASS for `(2,256)`, `(5,4096)`, `(3,8192)`, minimum counts, invalid shape, and invalid device-mode counts.
- The runner reports source paths and SHA values for both revisions. The build script verifies staged source copies against the expected values. The identity and plan commands do not call ACL. The invalid `--run` argument case exited before ACL setup; no valid device sequence, correctness rerun, or timing run was made.
- Device sequence, for a future authorized window only: parent warmup >=10; two in-process Parent blocks with >=21 device-event samples per block; report full-sample MAD/median and block drift; enter >=4 alternating PC/CP pairs only when both limits are <=0.10. Each pair block has >=21 device-event samples. Keep stdout raw samples with the Route evidence when the sequence is eventually run.
- Build logs, including failed attempts, are retained in `support/paired-runner/`; the successful log is `build-and-host-tests-attempt-8.log`.
- Research review: `phase4/research/MODE-X-R015C/next-hypotheses.md` retains four distinct research-only items. No candidate source changed.

## Current handoff state

Main decision remains `NEEDS_ONE_MORE_LOCAL`. Keep R015C-r4 unchanged. The runner and documentation work is complete; no device access, correctness rerun, timing, r2 comparison, or new revision was performed. Return these Route-owned records to Main review.

## Correctness provenance follow-up (2026-09-26)

- `support/server3/correctness.log` records exact PASS results for `(2,256)`, `(5,4096)`, and `(3,8192)`, but does not retain the commands or the executable identity observed during that run.
- `support/server3/build-attempt-2.log` identifies the compiled host source as `/home/data4t2/lelinfeng/MODE-X-R015C_runs/R015C-r4/source/op_host/row_copy_host.asc`. That source includes the adjacent R4 kernel; the staged kernel and tiling identities, the retained R4 sources, and the current server executable identity agree. This supports the R4 source lineage, but cannot independently prove which executable was launched for the historical correctness run.
- `support/server3/run-exact-correctness.sh` now verifies the expected host, kernel, tiling, and executable identities and prints the exact commands for all three shapes. `--plan-only DEVICE_ID` does not initialize ACL; `--run DEVICE_ID` runs the three targeted exact comparisons without rebuilding. A fresh device preflight remains necessary before using run mode.
- Plan attempt 1 matched all four identities but stopped while sourcing `set_env.sh` under `nounset`; the output is retained in `support/server3/exact-harness-plan-attempt-1.log`. Plan attempt 2 passed remotely, emitted the three exact device-probe commands, and ended with `NPU_ACCESS=NOT_RUN`; output is retained in `support/server3/exact-harness-plan-attempt-2.log`.
- This follow-up used plan-only/host-side validation only. No ACL or NPU operation, correctness rerun, or timing was performed.
