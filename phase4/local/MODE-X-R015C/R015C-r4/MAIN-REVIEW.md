# MODE-X-R015C R015C-r4 Main Review

Date: 2026-09-25

## Decision

`NEEDS_ONE_MORE_LOCAL`. Keep R015C-r4 unchanged. There is no performance conclusion, promotion, rejection, or Online submission. Do not create another performance revision while r4 is pending.

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

- Candidate executable SHA-256: `21934a8cf15508b365e252810c8f858b4fb105c5cc5000f460d61c5119aeeb390`.
- The recorded r3 executable is `faea8be543277b77af2bc07d6b08ca43134fceaed17ab807fe8cfd5ecdcd0aee`; a unified Parent/Candidate measurement runner has not yet been built and linked.
- No timing was run. The next local step is to prepare one runner whose Parent and Candidate paths are tied to their exact sources and executable identities. Device timing still requires a current MAIN-1 exclusive lease, then per-shape same-binary qualification before any pair.

## Documentation drift

The Route workspace `README.md` describes two rows per block and `DataCopy`. R015C-r4 uses one row per block and `DataCopyPad`. The Route owner should update that existing README to describe the current Route source, without changing r4.
