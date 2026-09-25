# MODE-X-R015C R015C-r4 Handoff

## Current state

- Approved hypothesis retained: one row per block; segment size, DataCopyPad path and order, and 64 KiB VECCALC scratch remain unchanged.
- Source SHA-256: `9367db4ebb4edf6b7bf6cde97f4846f987e6ab230c2aeee44d942d5e881a1c74`.
- Direct parent: R015C-r3, source SHA-256 `e1786bec2673519f41887fdffa0e11751f515a81037d854c88ae7edc0e4af903`.
- Compile and link: PASS on `cann-server3` (`hwnput3`), CANN `8.5.0.alpha002`, target `dav-2201`.
- Earlier correctness run: PASS on device 4 for `(2,256)`, `(5,4096)`, and `(3,8192)`; each output matched the input exactly. The identity-bound device-7 rerun below is the current correctness evidence.
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
- Device sequence, for a future authorized window only: use 45 or more warmups under the current protocol; two in-process Parent blocks with >=21 device-event samples per block; report full-sample MAD/median and block drift; enter >=4 alternating PC/CP pairs only when both limits are <=0.10. Each pair block has >=21 device-event samples. A fresh exclusive MAIN-1 lease is required. Keep stdout raw samples with the Route evidence when the sequence is eventually run.
- Build logs, including failed attempts, are retained in `support/paired-runner/`; the successful log is `build-and-host-tests-attempt-8.log`.
- Research review: `phase4/research/MODE-X-R015C/next-hypotheses.md` retains four distinct research-only items. No candidate source changed.

## Handoff state at 2026-09-25

Main decision remains `NEEDS_ONE_MORE_LOCAL`. Keep R015C-r4 unchanged. The runner and documentation work is complete; no device access, correctness rerun, timing, r2 comparison, or new revision was performed. Return these Route-owned records to Main review.

## Correctness provenance follow-up (2026-09-26)

- `support/server3/correctness.log` records exact PASS results for `(2,256)`, `(5,4096)`, and `(3,8192)`, but does not retain the commands or the executable identity observed during that run.
- `support/server3/build-attempt-2.log` identifies the compiled host source as `/home/data4t2/lelinfeng/MODE-X-R015C_runs/R015C-r4/source/op_host/row_copy_host.asc`. That source includes the adjacent R4 kernel; the staged kernel and tiling identities, the retained R4 sources, and the current server executable identity agree. This supports the R4 source lineage, but cannot independently prove which executable was launched for the historical correctness run.
- `support/server3/run-exact-correctness.sh` now verifies the expected host, kernel, tiling, and executable identities and prints the exact commands for all three shapes. `--plan-only DEVICE_ID` does not initialize ACL; `--run DEVICE_ID` runs the three targeted exact comparisons without rebuilding. A fresh device preflight remains necessary before using run mode.
- Plan attempt 1 matched all four identities but stopped while sourcing `set_env.sh` under `nounset`; the output is retained in `support/server3/exact-harness-plan-attempt-1.log`. Plan attempt 2 passed remotely, emitted the three exact device-probe commands, and ended with `NPU_ACCESS=NOT_RUN`; output is retained in `support/server3/exact-harness-plan-attempt-2.log`.
- This follow-up used plan-only/host-side validation only. No ACL or NPU operation, correctness rerun, or timing was performed.

## Exact-source correctness rerun on device 7 (2026-09-26)

- The retained R4 submission, kernel, tiling, host wrapper, and server executable matched their recorded identities before invocation. Exact values and full command/output are retained in `support/server3/correctness-device7-exact-20260926.log`.
- Invocation: `ssh cann-server3 bash -s -- --run 7`, with `run-exact-correctness.sh` on stdin. The script rechecked all four identities, then ran only the three fixed `--device-probe` exact comparisons; it has no build, warmup, event, sampling, or timing branch.
- Immediately before the run, device 7 was healthy, AICore was 0%, and HBM use was `14394/65536 MB` (51142 MB free). Its listed process was PID `1300597`, `python`, 11016 MB. The command was separately inspected before the run and was `build_minicpmo_cremad_reference.py`, not a correctness/probe task.
- All three cases passed exact comparison: `(2,256)`, `(5,4096)`, `(3,8192)`; run exit status was 0.
- Immediately after, device 7 remained healthy, AICore was 0%, and HBM use was `14537/65536 MB`; the same PID remained listed at 11012 MB. A post-run process query at `2026-09-25T20:01:16Z` confirmed the same unrelated command.
- The full log's two derived `DEVICE7_PROCESS` summary lines have shifted columns because the `npu-smi` row starts with an empty field. Do not use those two summaries: the raw pre/post device tables in the same log correctly show PID `1300597`, process `python`, and memory `11016` / `11012 MB`. The standalone process query and corrected interpretation are retained in `support/server3/correctness-device7-process-audit-20260926.log`.
- No timing, same-binary qualification, or performance sampling was performed.
- Main confirmed this identity-bound result: kernel source SHA-256 `9367db4ebb4edf6b7bf6cde97f4846f987e6ab230c2aeee44d942d5e881a1c74`, tiling SHA-256 `0939ba8498426fcd77645d826fcc45a8eb65c78996ad0ce1da1a00d1ac01a250`, and executable SHA-256 `21934a8cf15508b365e252810c8f858b4fb105c5cc5000f460d61c511a37e188` all match the run record. The three requested shapes passed exact comparison with exit status 0. Same-binary qualification and paired timing remain outstanding; Main retains `NEEDS_ONE_MORE_LOCAL`.

## Current handoff state (2026-09-26)

- Main-confirmed exact-source correctness is PASS for the three recorded shapes and the executable identity above.
- Main decision remains `NEEDS_ONE_MORE_LOCAL`; the exact-source correctness result and all four source/executable identities remain unchanged.
- R015C-r2 remains `DIAGNOSTIC_ONLY_CORRECTNESS_FAIL`; it was not compared with r3 or r4. An R015C-r2 evidence directory is absent from this Route worktree, so this status is retained from the current Main handoff.
- Current device report supplied for this handoff: d0-d6 have resident VLLM processes and approximately 90%-92% HBM use. d7 is idle but excluded by the timing protocol. The shared lease table has no R015C lease. Performance status is `MEASUREMENT_BLOCKED`; same-binary qualification, paired timing, and performance load classification remain outstanding.
- Track-B screening retains three distinct research-only ideas, H08-H10, with shape probes and falsification conditions in `phase4/research/MODE-X-R015C/next-hypotheses.md`. No Candidate, runner, NPU, or timing work was performed in this continuation.
