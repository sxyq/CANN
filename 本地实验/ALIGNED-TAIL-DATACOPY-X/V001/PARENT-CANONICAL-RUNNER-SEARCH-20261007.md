# Parent Canonical Correctness Runner Search

- Route: `ALIGNED-TAIL-DATACOPY-X`
- Revision under test: unchanged Parent source; SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Worktree: `/home/data4t2/lelinfeng/cann-r-w4-5-aligned-tail`
- Result: `NO_CANONICAL_PARENT_RUNNER`
- Existing classification: `PARENT_HARNESS_OR_BASELINE_BLOCKED`

## Bounded search

Read-only inventory covered the project `工具/` directory, the official
`线上结果/R31B/V011/` artifacts, the champion reference, and project
`本地实验/` entries that mention the exact V011 source SHA or Parent
correctness. No canonical, already-used Parent full-correctness entry or
runner executable is present. The official V011 directory contains submission
source, source/result metadata, checksum, and diff only. Project tools document
CANNJudge submission and read-only result queries, not a local correctness
runner.

## Existing non-canonical probe excluded

`本地实验/R31B/V016/correctness-evidence.txt` records a V016 route-local
FP32 probe compiled against the exact V011 source SHA. Its recorded results
include `2x8192 PASS` and `2x16384 FAIL` on device 7. The same note says the
failure root cause remains unresolved and that no independent AddRmsNormBias
oracle was run. Its CMake probe configuration references a separate
R31B-V011 source directory, and its recorded executable/build path is outside
this worktree. This is existing direct-runner evidence, not an authoritative
Parent correctness entry, so it was not rerun or treated as a canonical gate.

## Route result

No canonical comparison was run. The previous route-local full Parent suite
remains failed on six FP32-wide shapes; width 8192 passed and width 8193 and
larger failed. No harness/source difference can be attributed from a
canonical comparison because no such entry was found.

No Candidate, Parent source, harness, build configuration, or shared record
was changed. No new V001, Compile, Local, or Online action was performed.
Retain `PARENT_HARNESS_OR_BASELINE_BLOCKED` and wait for Planning or a new,
verifiable canonical Parent correctness entry.
