# MODE-X-R015C R015C-r4 Main Review

Date: 2026-09-25
Updated: 2026-09-26

## Decision

No local performance verdict has been formed. Keep R015C-r4 unchanged. There is no performance conclusion, promotion, rejection, or Online submission. Do not create another performance revision while r4 is unresolved. `BUILD=PASS`; `EXECUTABLE_IDENTITY=PASS`; the recorded 3/3 correctness run covers only the row-copy microkernel, so complete AddRmsNormBias correctness is `INCOMPLETE`. The route is not ready for same-binary or timing.

## Lineage and change scope

- Route: `MODE-X-R015C`
- Revision: `R015C-r4`
- Direct Parent: `R015C-r3`
- Parent source SHA-256: `e1786bec2673519f41887fdffa0e11751f515a81037d854c88ae7edc0e4af903`
- Candidate source SHA-256: `9367db4ebb4edf6b7bf6cde97f4846f987e6ab230c2aeee44d942d5e881a1c74`
- Context: `HISTORICAL_DERIVED`
- Hypothesis: assign one row to each block. Preserve segment size, copy path and order, barriers, and 64 KiB staging allocation.
- `SINGLE_CHANGE_AUDIT=PASS`. The source diff changes row-to-block mapping and retains a two-row-sized staging allocation; no separate arithmetic, copy, or synchronization optimization was found.

## API and implementation evidence

- The installed CANN package has no local API Markdown tree. The review used the CANN 8.5.0.alpha002 Ascend C headers on `cann-server3`, plus the repository's local API guidance.
- The DAV_2201 `DataCopyPad` implementation accepts `DataCopyExtParams`; its checked block-length limit is 2,097,151 bytes. This kernel uses one block per copy and at most 8,192 bytes per segment.
- The installed interfaces expose `DataCopyPad` on MTE2 and MTE3 and `PipeBarrier<pipe>`. The kernel places a full barrier after each GM-to-local copy and each local-to-GM copy before reusing staging.
- The route host validation limits D to 256..8192 and requires divisibility by 8. Segment lengths are therefore whole FP32 elements, and the largest row is transferred in one 8,192-byte segment.
- Compile and link passed on CANN `8.5.0.alpha002`, Ascend910B3, `dav-2201`. Exact output correctness passed for `(2,256)`, `(5,4096)`, and `(3,8192)`.

## Measurement readiness

- Candidate standalone executable SHA-256: `21934a8cf15508b365e252810c8f858b4fb105c5cc5000f460d61c511a37e188`, matching `source-meta.json` and `local-result.json`.
- The unified Parent/Candidate runner built and linked on server3 from the declared sources. Runner SHA-256: `0b576bea88cc5c6664728f3e5a1f749e1560f189fab9b6b714fd2cbd578c5837`; Parent module SHA-256: `8f57bc1b028f815e4f1cea53f5b5d22781bddb6077b7cc3a47acf70123ab4486`; Candidate module SHA-256: `9339fd686d9498fc55d899aa0ef6dac6c49a7092b93b0b36e7a38eaed6f66b33`.
- Host identity and argument tests passed for the three declared shapes and invalid count/device-mode arguments. They did not initialize ACL or access an NPU.
- The paired runner has not been used for device qualification, correctness, or timing. The separate standalone exact-correctness run recorded below is identity-bound. A future measurement requires a fresh exclusive MAIN-1 lease and exact-shape same-binary qualification before any Parent/Candidate pair.
- The current protocol calls for 45 or more warmups, two in-process Parent blocks with at least 21 device-event samples per block, and both full-sample MAD/median and block drift at or below 0.10 before at least four alternating PC/CP pairs. The shared lease record has no active R015C lease.

## Documentation alignment

The earlier README drift is resolved in `phase4/workspaces/MODE-X-R015C/README.md`: it now describes one row per block, `DataCopyPad`, the 64 KiB staging buffer, and the current runner. R015C-r4 source remains unchanged.

## Identity-bound correctness confirmation (2026-09-26)

Main confirmed exact-source NPU correctness PASS for `(rows=2,D=256)`, `(rows=5,D=4096)`, and `(rows=3,D=8192)`, using the retained R4 executable SHA-256 `21934a8cf15508b365e252810c8f858b4fb105c5cc5000f460d61c511a37e188`.

- Invocation recorded in `support/server3/correctness-device7-exact-20260926.log`: `ssh cann-server3 bash -s -- --run 7`, with `run-exact-correctness.sh` supplied on stdin. The script verified the host wrapper SHA-256 `fbd3147221a36756d9fd65656635a4d5c92e8ad8be77405ed75c8de97e8726de`, kernel source SHA-256 `9367db4ebb4edf6b7bf6cde97f4846f987e6ab230c2aeee44d942d5e881a1c74`, tiling SHA-256 `0939ba8498426fcd77645d826fcc45a8eb65c78996ad0ce1da1a00d1ac01a250`, and executable SHA-256 above before invoking the three fixed probes.
- Each exact comparison passed; `CORRECTNESS_RUN_EXIT_STATUS=0`. No build, warmup, sampling, same-binary run, or timing branch was used.
- Device 7 preflight at `2026-09-25T19:59:31Z`: health OK, AICore 0%, HBM `14394/65536 MB`; post-run at `2026-09-25T19:59:50Z`: health OK, AICore 0%, HBM `14537/65536 MB`. The raw process rows show PID `1300597` (`python`) at `11016` then `11012 MB`; `support/server3/correctness-device7-process-audit-20260926.log` identifies it as an unrelated workload. The wrapper's derived process summary lines are malformed and are excluded.
- Exact command, identities, raw pre/post tables, and case output are retained in `support/server3/correctness-device7-exact-20260926.log`; process interpretation is retained in `support/server3/correctness-device7-process-audit-20260926.log`.

The identity gap for correctness is closed. Same-binary qualification and paired timing remain absent, so the Main decision remains `NEEDS_ONE_MORE_LOCAL`; load quality for performance has not been assessed, and no timing is authorized without a fresh lease.
