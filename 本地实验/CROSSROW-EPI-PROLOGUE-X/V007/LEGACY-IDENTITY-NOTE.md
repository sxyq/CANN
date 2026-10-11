# V007 legacy evidence identity separation

V007 has two preserved identity batches. The earlier files are not evidence for the corrected Candidate.

| Evidence | Source SHA | Artifact SHA | Status |
|---|---|---|---|
| `build-v007-canonical-20261011.log` | `320436ef3364be8de6b8ca2bb4dd62c7dfcf17e48f0b690de0c217e2678fa3d4` | Candidate `7e9419927b1615847127a4ce0c6ad1012c4814c30f2adbb46d92e4ee51d39631` | Old placement; build passed, excluded from corrected Candidate identity |
| `canonical-correctness-20261011/` | `320436ef3364be8de6b8ca2bb4dd62c7dfcf17e48f0b690de0c217e2678fa3d4` | Same old build batch | 14/14 RC=0 retained, excluded from corrected Candidate correctness |
| `build-v007-c09344-canonical-20261011.log` | `c09344af4e9ffb78ca50bf8f23f75a6dca1e24640b803ef1d415777cb46381b5` | Candidate `7f7a521b869b9a1e3eaefb23ef9c3daff9853c12aa5002b047d9f568cf9ffcf0` | Corrected placement; Build PASS |
| `canonical-correctness-c09344-20261011/` | `c09344af4e9ffb78ca50bf8f23f75a6dca1e24640b803ef1d415777cb46381b5` | Same corrected build batch | 14/14 RC=0; PASS_7_OF_7 |
| `canonical-paired-c09344-20261011/` | `c09344af4e9ffb78ca50bf8f23f75a6dca1e24640b803ef1d415777cb46381b5` | Same corrected build batch | 24/24 valid Local side runs; scored below |

The old raw/log files were not overwritten or removed. The current declaration and result use only source SHA `c09344af…` and artifact SHA `7f7a521b…` for Candidate claims.
