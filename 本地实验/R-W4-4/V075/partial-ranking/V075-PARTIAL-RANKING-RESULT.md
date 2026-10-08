# R-W4-4 V075 Partial Ranking Result

- `REVISION=V075`; exact comparison Parent is `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate source SHA256: `eb660bcd633cfc2df482c08d3cdb0c913b22b16d7887f7a1e0e3eda1ace3ffb3`.
- Single OFAT change: `kSmallFp32BatchMaxWidth`, `4096 -> 8320`; the Candidate differs from exact V011 only at this constant.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact-Parent C15 FP32 `1x32768`; excluded and not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Compile and Correctness

Route build passed (`ROUTE_BUILD_RC=0`) and the `clx_ref_parent_probe` / `clx_ref_candidate_probe` reference-harness targets built successfully (`REF_PROBE_BUILD_RC=0`). Correctness was run on fixed device 4 for only the authorized FP32 shapes; C15 was not run.

| FP32 shape | Exact Parent | V075 Candidate | Parent bad | Candidate bad | Local |
|---|---|---|---:|---:|---|
| `128x8312` | FAIL, `rc=3` | FAIL, `rc=3` | `539077` | `538369` | skipped: Parent invalid |
| `128x8320` | FAIL, `rc=3` | FAIL, `rc=3` | `317734` | `525052` | skipped: Parent invalid |
| `128x8328` | FAIL, `rc=3` | FAIL, `rc=3` | `545082` | `422304` | skipped: Parent invalid |

No tested shape has a valid exact Parent, so there is no eligible Parent/Candidate Local comparison. The paired failures do not establish a Candidate regression.

## Result Flags

- `COMPILE=PASS`; reference probe build `PASS`.
- `CORRECTNESS=BLOCKED_PARENT_FAILURE`: exact Parent failed all three tested widths.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL=NOT_RUN_NO_PARENT_VALID_SHAPE`.
- `LOCAL_SCORE=NONE`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.
- `ONLINE=NOT_ELIGIBLE`; `CURRENT_LOCAL_BEST=NONE` remains unchanged. Use exact `R31B-V011` for the next authorized threshold OFAT.
- Device 4 reservation for V075 is released after completion of the correctness evidence capture.
