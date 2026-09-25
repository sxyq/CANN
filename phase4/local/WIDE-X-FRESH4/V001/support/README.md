# WIDE-X-FRESH4 V001 Unified Runner

The runner compares the exact V001 Candidate with its direct Parent, `BUILD-FIX-001`, from one process and one shared input allocation. It does not alter either source.

## Identity

- Parent source: `../../BUILD-FIX-001/submission.asc`, SHA256 `5d0ee01165e46a281cbb7d1605feba3605ad59883845400a063e97704f27f2be`.
- Candidate source: `../submission.asc`, SHA256 `f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895`.
- Input generator: `WIDE_X_FRESH4_NPU_CORRECTNESS_GEN1_SEED0`; rows=2, epsilon=1e-5, availableCoreNum=40. Input formulas match the Route's NPU correctness runner.
- Supported cases: widths 2048, 16384, 32768; dtypes `fp32`, `fp16`, `bf16`.

## Measurement

The process allocates device buffers and copies inputs once, then uses one stream for warmup and all blocks. Each warmup launch is followed by a stream sync. Each measured launch records start/stop device events and synchronizes the stop event; `device_event_us` is primary. `wall_us` measures host launch through event synchronization and is diagnostic only. No allocation, H2D, or D2H occurs in the sample loop.

`same` mode runs one side for multiple in-process blocks. `paired` mode alternates adjacent Parent/Candidate launches and reverses the first side on alternating samples and blocks. The executable rejects fewer than 10 warmups, 21 samples per block, 2 same-side blocks, or 4 paired blocks. It creates the TSV with exclusive-create semantics, so an existing raw result is never overwritten.

On shutdown it synchronizes the stream before releasing ACL resources. If that drain fails, it reports the cleanup failure and skips event/buffer/stream destruction, device reset, and runtime finalization; the process retains its original nonzero result.

```text
wide_x_fresh4_unified_runner DEVICE WIDTH DTYPE MODE SIDE WARMUPS SAMPLES BLOCKS OUTPUT.tsv
```

For `same`, SIDE is `parent` or `candidate`; for `paired`, SIDE is `-`. Timing requires the current Main device lease and is not started by the build script.

## Build

Run `bash build_server3.sh` on cann-server3. The script verifies both source SHA256 values, uses `set_env.sh` when installed or the toolkit's CMake package path otherwise, builds and links the two symbol-renamed kernel libraries plus the shared runner, checks runtime dependencies, and prints artifact SHA256 values. It does not execute the runner.
