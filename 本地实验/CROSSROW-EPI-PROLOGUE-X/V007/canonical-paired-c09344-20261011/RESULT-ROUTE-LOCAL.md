# V007 MAIN2-CANONICAL-V1 route-local result

ROUTE: `CROSSROW-EPI-PROLOGUE-X`
REVISION: `V007`
SOURCE_CHECKPOINT_COMMIT: `9c6d19fae1373a814a1eb044d7780cac7c2c489a` (initial snapshot)
SOURCE_CORRECTION_COMMIT: `901ecf4c5f30f4761dc6d0ad921e9a7a8071a11d`
DIRECT_PARENT: `R31B/V011` reliable baseline; V006 was not used as Parent or Best
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
CANDIDATE_SOURCE_SHA256: `c09344af4e9ffb78ca50bf8f23f75a6dca1e24640b803ef1d415777cb46381b5`
PARENT_ARTIFACT_SHA256: `e5d05ac104c99450904a4dda4417d6d5ecbd50d04412bc129622b51a56e2cf64`
CANDIDATE_ARTIFACT_SHA256: `7f7a521b869b9a1e3eaefb23ef9c3daff9853c12aa5002b047d9f568cf9ffcf0`
RUNNER_REF_SHA256: `2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7`

## Build and Correctness

Build PASS RC=0 for the corrected Candidate SHA; exact command, toolkit HCC C++ include-path environment fix, artifact identity, and selector restore are in `build-v007-c09344-canonical-20261011.log`. The earlier `320436ef…` build and its 14 correctness RCs are preserved and explicitly excluded in `LEGACY-IDENTITY-NOTE.md`.

Correctness PASS_7_OF_7: C01/C08/C11/C12/C13/C14/C16, Parent and Candidate, 14/14 RC=0, `bad=0`, using the `c09344af…` Candidate artifact. Raw/log/RC/stats and device snapshots are in `canonical-correctness-c09344-20261011/`.

## Local protocol and Core summary

Device 7, four Cores C12/C13/C14/C16, three `P-C/C-P/P-C` pairs per case, 45 warmups and 31 device-event samples per side, blocks=1, gap=0, batch_n=1. All 24 side runs RC=0, bad=0; all 744 event samples are retained. Same-binary qualification was NOT_RUN.

| Core | Anchor us | Parent run medians us | Candidate run medians us | Parent median | Candidate median | Candidate faster pairs | Candidate quality | Cell quality |
|---|---:|---|---|---:|---:|---:|---|---|
| C12 | 6.58 | 18.30 / 18.32 / 18.50 | 20.38 / 18.50 / 15.58 | 18.32 | 18.50 | 1/3 | POOR | POOR |
| C13 | 20.80 | 31.82 / 24.56 / 62.24 | 34.66 / 27.30 / 27.60 | 31.82 | 27.60 | 1/3 | FAIR | POOR |
| C14 | 25.96 | 26.14 / 25.68 / 41.60 | 25.08 / 70.32 / 24.56 | 26.14 | 25.08 | 2/3 | POOR | POOR |
| C16 | 15.06 | 48.74 / 15.44 / 25.44 | 25.06 / 15.34 / 15.42 | 25.44 | 15.42 | 3/3 | POOR | POOR |

Canonical score: `72.149204`; delta vs frozen R31B/V011: `-27.850796`; paired Parent diagnostic: `118.369252`. Quality is POOR; verdict is LOCAL_NEUTRAL, NO_PROMOTION, Local Best unchanged. The Candidate beats the anchor only on C14; paired medians favor Candidate in 3/4 Cores, but paired Parent is substantially slower than the frozen anchor overall.

## Load / contamination

NPU7 pre/post HBM usage was 37% (roughly 41 GB free; integer usage only, so no exact free value claimed); pre AICore/AIVector were 0%/0%, post 0%/0%. `npu-smi info proc -i 7` reports process querying unsupported. Process cleanliness is UNKNOWN; no clean-device claim. No execution/resource error occurred.

## Deduplication note

The current Candidate implements its declared R1-local single change and is distinct from R1 V001-V006. A read-only check of published `origin/w2/main2/control` at `6d3ad0b…` shows ASYNC-OVERLAP-CHAMPION-X V001 publicly records the same pass-2 gamma/bias MTE2 hoist above the invRms scalar loop. Thus V007 is not globally novel against that published mechanism; this overlap is disclosed and no Local Best promotion is claimed. No other Route private source was read.
