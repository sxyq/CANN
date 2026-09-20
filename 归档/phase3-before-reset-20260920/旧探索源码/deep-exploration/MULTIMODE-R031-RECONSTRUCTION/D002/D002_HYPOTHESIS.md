# D002 hypothesis

## Hypothesis

D001 selected batch modes with a ceiling estimate of rows per core. D002 tests the single boundary hypothesis that batch selection should use the floor average `avgRows = rowCount / blockCount`, matching the mechanism description. A batch mode is enabled only when the average work per core reaches its threshold; uneven tails therefore remain on the single-row plan. This keeps the FP32 two-pass arithmetic chain and every tile unchanged.

## Plans

| Plan | Resolver condition | Tile | Rows per batch | Parameter policy |
| --- | --- | ---: | ---: | --- |
| `SMALL_FEW` | `D <= 1024` or `avgRows <= 1` | 1024 | 1 | load per row |
| `MEDIUM_MANY` | `D <= 4096` and `avgRows >= 4` | 2048 | 4 | reuse within batch |
| `WIDE_MANY` | `D > 8192` and `avgRows >= 2` | 4096 | 2 | reuse within batch |
| `LARGE_FEW` | all other aligned rows | 4096 | 1 | load per row |
| `UNALIGNED` | row width is not a 32-byte multiple | 1024 | 1 | load per row |

The `UNALIGNED` plan has priority over the other plans and uses valid-byte `DataCopyPad` lengths for both input and output. The five plans cover FP16, BF16, and FP32 through separate template instantiations. BF16 arithmetic is converted to FP32 before vector operations and converted back only at output. D002 changes only the resolver boundary from D001.

## Compile acceptance

The D002 acceptance point is a successful CANN 9.0 compile on server3. No local NPU execution, matrix validation, profiling, benchmark, or platform submission is part of this candidate.
