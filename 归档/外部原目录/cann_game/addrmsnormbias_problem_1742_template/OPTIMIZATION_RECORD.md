# AddRmsNormBias Optimization Record

## Scope and measurement

- Production change: `kernel.asc`. The record is the only accompanying non-kernel file.
- Target used for local validation: Ascend 910B3, CANN 9, 40 vector cores.
- Large-shape measurements use 500 repeats. A change is accepted only after a
  candidate-baseline-candidate (C-B-C) comparison and a correctness pass.
- Times below are local NPU measurements. They are useful for attribution but
  are not a substitute for an official leaderboard submission.

## Retained optimizations

| Path | Change | C-B-C local result | Attributed gain |
| --- | --- | --- | --- |
| FP32, D=4096 | Batch full-tile rows and overlap the two output stores. | 26.325 / 27.150 / 26.366 us | 2.96% faster |
| FP32, D=8192 | Cache one full row and pipeline alternating output buffers. | 46.703 / 53.790 / 46.179 us | 13.66% faster |
| FP16, D=4096 | Batch two full-tile rows with alternating output buffers. | 22.038 / 26.572 / 22.052 us | 17.04% faster |
| FP16, D=8192 | Cache one full row and pipeline alternating output buffers. | 38.437 / 49.502 / 38.202 us | 22.59% faster |
| BF16, D=4096 | Batch two full-tile rows and reuse parameter staging as output buffers. | 27.297 / 32.870 / 27.491 us | 16.66% faster |
| BF16, D=8192 | Cache one full row and reuse parameter staging as output buffers. | 49.171 / 59.774 / 48.765 us | 18.08% faster |

The retained implementation also preserves the preceding path-specific work:

- FP32 direct parameter caching and batched parameter reuse on wide rows.
- FP16 native add/multiply/add output arithmetic, avoiding unnecessary FP32
  parameter conversion on that path.
- BF16 parameter conversion once per core when it can be reused across rows.
- Cached-row and contiguous-row specializations for small and wide shapes.
- Double-buffered reduction/output work for wide FP32 and low-precision paths.
- A tuned FP32 wide-row batch tile size of 6112 elements.

## Officially retained boundary path

The D=8193 cache-boundary path is retained. It moves this one width out of the
wide path and uses 8208-element, 32-byte-aligned caches while leaving the
fixed-width D=8192 specializations unchanged.

An isolated local C-B-C run at M=512 produced mixed results:

| Type | Candidate 1 | Baseline | Candidate 2 |
| --- | ---: | ---: | ---: |
| FP32 | 36.761 us | 33.690 us | 36.700 us |
| FP16 | 34.669 us | 30.373 us | 34.357 us |
| BF16 | 41.285 us | 42.298 us | 40.874 us |

Despite those local measurements, the supplied version containing this path
received an official score of **47.01** at `2026-09-08 07:55:49`, improving the
previous official 41.83 by 5.18 points (12.4%). The hidden workload mix and
scoring weights therefore favor retaining it; the official result takes
precedence over this isolated local comparison.

## Rejected experiments

Other rejected directions were fused multiply-add substitutions, routine tile
prefetching, vector reciprocal-square-root precision changes, larger 16-row
batches, unsafe output prefetching, and a full FP32 cache beyond D=4096. The
last of these also triggered device error 507035. The available register API is
not supported by this target. A BF16 wide-path promotion experiment either
failed correctness or was slower, so none of these are present in `kernel.asc`.

## Final local regression

The final expanded NPU run passed all exercised FP32, FP16, and BF16 cases,
including irregular widths and D=32768. Representative 500-repeat results for
the retained specializations were:

| Shape | FP32 | FP16 | BF16 |
| --- | ---: | ---: | ---: |
| M=1024, D=4096 | 26.295 us | 21.732 us | 27.387 us |
| M=1024, D=8192 | 46.117 us | 38.180 us | 48.702 us |

The best reported official score for the retained source is 47.01. The
theoretical reference score remains separate from local timing and should not
be inferred from these measurements.
