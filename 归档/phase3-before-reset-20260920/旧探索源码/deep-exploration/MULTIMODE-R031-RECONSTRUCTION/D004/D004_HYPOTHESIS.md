# D004 hypothesis

## Hypothesis

For FP16 and FP32, a non-32-byte-aligned row width should not force the smallest single-row plan. D004 removes the alignment predicate from the front of the resolver: it first selects `SMALL_FEW`, `MEDIUM_MANY`, `WIDE_MANY`, or `LARGE_FEW` from D and the floor average `avgRows`, then relies on `DataCopyPad` valid-byte lengths for the tail. BF16 retains D003's unconditional `LARGE_FEW` single-row boundary.

The expected benefit is that an unaligned row can still use a larger tile or multi-row parameter reuse when its D and row workload justify it. The change is restricted to the alignment-to-plan boundary; the reduction formula, FP32 middle path, tile loop, and valid-element copy handling are unchanged.

## Resolver behavior

| dtype | Boundary rule | Alignment handling |
| --- | --- | --- |
| FP32 | D/`avgRows` classifier | Unaligned widths follow the selected plan |
| FP16 | D/`avgRows` classifier | Unaligned widths follow the selected plan |
| BF16 | Unconditional `LARGE_FEW` | Unaligned widths remain single-row |

For any selected plan, each input and output tile uses its valid element count and `DataCopyPad`; no testcase index is assigned a hidden shape or dtype.

## Acceptance

The acceptance point for D004 is a successful CANN compile on server3. No local NPU execution, CPU matrix, profiling, benchmark, or platform submission is part of this candidate.
