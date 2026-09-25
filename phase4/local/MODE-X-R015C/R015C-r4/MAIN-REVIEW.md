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

- Candidate standalone executable SHA-256: `21934a8cf15508b365e252810c8f858b4fb105c5cc5000f460d61c511a37e188`, matching `source-meta.json` and `local-result.json`.
- The unified Parent/Candidate runner built and linked on server3 from the declared sources. Runner SHA-256: `0b576bea88cc5c6664728f3e5a1f749e1560f189fab9b6b714fd2cbd578c5837`; Parent module SHA-256: `8f57bc1b028f815e4f1cea53f5b5d22781bddb6077b7cc3a47acf70123ab4486`; Candidate module SHA-256: `9339fd686d9498fc55d899aa0ef6dac6c49a7092b93b0b36e7a38eaed6f66b33`.
- Host identity and argument tests passed for the three declared shapes and invalid count/device-mode arguments. They did not initialize ACL or access an NPU.
- The runner has not been invoked for same-binary qualification, correctness, or timing. A future measurement requires a current MAIN-1 exclusive lease and exact-shape same-binary qualification before any Parent/Candidate pair.

## Documentation drift

The Route workspace `README.md` describes two rows per block and `DataCopy`. R015C-r4 uses one row per block and `DataCopyPad`. The Route owner should update that existing README to describe the current Route source, without changing r4.
