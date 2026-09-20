# D001 hypothesis

## Hypothesis

A runtime workload classifier can select a small set of bounded execution plans using only `dtype`, `D`, aligned row width, and the estimated rows assigned to one Vector Core. Small and medium rows should benefit from processing several rows while reusing `gamma` and `bias`; large rows should use a 4096-element channel tile with one-row scheduling to keep UB usage bounded. Every plan keeps the same FP32 two-pass arithmetic chain, so the classifier changes data movement and scheduling without changing the numerical formula.

## Plans

| Plan | Resolver condition | Tile | Rows per batch | Parameter policy |
| --- | --- | ---: | ---: | --- |
| `SMALL_FEW` | `D <= 1024` or estimated rows per core <= 1 | 1024 | 1 | load per row |
| `MEDIUM_MANY` | `D <= 4096` and estimated rows per core >= 4 | 2048 | 4 | reuse within batch |
| `WIDE_MANY` | `D > 8192` and estimated rows per core >= 2 | 4096 | 2 | reuse within batch |
| `LARGE_FEW` | all other aligned rows | 4096 | 1 | load per row |
| `UNALIGNED` | row width is not a 32-byte multiple | 1024 | 1 | load per row |

The `UNALIGNED` plan has priority over the other plans and uses valid-byte `DataCopyPad` lengths for both input and output. The five plans cover FP16, BF16, and FP32 through separate template instantiations. BF16 arithmetic is converted to FP32 before vector operations and converted back only at output.

## Compile acceptance

The D001 acceptance point is a successful CANN 9.0 compile on server3. No local NPU execution, matrix validation, profiling, benchmark, or platform submission is part of this candidate.
