# V028 C15 correctness recheck — device 2

- ROUTE / REVISION: MODE-DISPATCH-CUTOFF-X / V028
- PURPOSE: correctness-only cross-device confirmation of the committed C15 failure; not Local performance measurement.
- EXECUTABLES: existing `clx_ref_parent_probe` and `clx_ref_candidate_probe` from this worktree's V028 staged correctness harness.
- COMMAND SHAPE: device=2, rows=1, width=32768, dtype=0 (FP32), warmup=0, samples=1, blocks=1, gap=0, batch_n=1; Parent then Candidate sequentially.
- RUN INTERVAL: resource precheck at `2026-10-06T22:10:01Z`; probe stdout/stderr mtimes span `22:10:36Z`–`22:10:54Z`; post snapshot at `22:10:56Z`.
- RESULT: Parent `rc=3`, bad=28651, max_abs=1.2031; Candidate `rc=3`, bad=28527, max_abs=1.2031. Both fail the C15 tolerance check.
- DEVICE / LOAD: 910B3 device 2; HBM 60210/65536 MB before and 60217/65536 MB after; AICore 88.8% before and 88.4% after; `VLLMWorker_TP` PID 92813, 56703 MB reported. No other process was changed.
- DISK: 411 GB available before and after (`df-pre.txt`, `df-post.txt`).
- INTERPRETATION: the failure reproduces on the assigned device with both exact source identities, so it is not specific to device 7. The check remains a correctness diagnostic only; its single-launch timings are not performance evidence. It still does not isolate a minimal kernel edit, and neither source passes correctness.
- DISPOSITION: retain `PARENT_SHARED_FAILURE`; no Local, no performance edit, Online forbidden. Next action requires a localized correctness diagnosis / Planning direction before resuming the route loop.
- ARTIFACTS: `parent-stats.txt`, `candidate-stats.txt`, raw TSVs, stdout/stderr, `status.txt`, and pre/post NPU and disk snapshots in this directory.
