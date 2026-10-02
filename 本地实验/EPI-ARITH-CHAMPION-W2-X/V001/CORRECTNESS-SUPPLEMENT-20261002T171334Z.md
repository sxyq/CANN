# EPI-ARITH-CHAMPION-W2-X V001 Correctness Supplement

- Candidate source SHA-256: `9a28f5cd8703dc4ff5c46fba09f59a537132594e5a879a50a17349086bfe4d59`
- Direct Parent: R31B V011, source SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Build: `PASS`; correctness runner SHA-256 `21f8f8e4f90fb007187092a7fede45e9f15b0132ccbc78b6e921985ef0c40539`; Parent library SHA-256 `1e81e41cbfb2a269a843dc089bcc68c2d56e3bbf4f31850714e5d3f5a2fe6c42`; Candidate library SHA-256 `ba9216ee7589100f6af072456dd1e3e02f8ad8aac9ed7be17eecfba77620590f`.
- Executable/source identities were confirmed against the existing build and fixed V001 / V011 source files. No source or build artifact was changed.
- Primary matrix run: `20261002T163852Z`, device 7, `rows=2`, FP32, `blockCount=1`.
- Authorized narrow retry: `20261002T171334Z`, device 7, D8193 only; one Parent call and one Candidate call, using the existing runner and binaries with fresh result paths.
- Overall correctness matrix: `FAIL`, not PASS. Of 14 side/shape entries, 2 PASS, 10 FAIL against the runner's CPU golden, and 2 remain MISSING for D8193.
- Local performance: not authorized and not run.

## Shape Results

Matched ratios and maximum absolute errors are from each side's TSV in the primary run. The precision limits were `atol=1.52587891e-05`, `rtol=0.0009765625`, required matched ratio `>=0.99`, and maximum absolute error `<=0.01`.

| D | Source-derived batch rows | Parent | Candidate | Attribution |
|---:|---:|---|---|---|
| 8192 | 0 (non-wide control) | PASS; ratio 1; max error 4.76837158e-07 | PASS; ratio 1; max error 4.76837158e-07 | Control path; the V001 arithmetic change is not selected. |
| 8193 | 2 predicted; no result row emitted | MISSING | MISSING | Parent and Candidate both returned ACL 507035 at stream synchronization. No output was compared. |
| 12288 | 2 | FAIL; ratio 0.240315755; max error 3.50248498 | FAIL; ratio 0.333536784; max error 3.26970172 | Both sides fail the same CPU golden limits. This does not isolate a Candidate-only failure. |
| 16384 | 2 | FAIL; ratio 0.191680908; max error 3.61958987 | FAIL; ratio 0.250610352; max error 3.07875907 | Both sides fail the same CPU golden limits. This does not isolate a Candidate-only failure. |
| 18416 | 2 | FAIL; ratio 0.166702867; max error 3.50157815 | FAIL; ratio 0.167191573; max error 3.07959926 | Both sides fail the same CPU golden limits, at the two-row budget boundary. |
| 18417 | 1 | FAIL; ratio 0.140278004; max error 16.1726532 | FAIL; ratio 0.115192485; max error 16.1726532 | Both sides fail after the source-derived batch changes to one row. |
| 32768 | 1 | FAIL; ratio 0.063369751; max error 12.5908301 | FAIL; ratio 0.069152832; max error 12.5908301 | Both sides fail with one row per batch. |

## D8193 Retry

Both calls used the already-built runner from the primary build directory and the fixed source identities above. The exact command forms were:

```text
.../epi_arith_v001_correctness 7 parent 8193 .../20261002T171334Z/correctness/parent-r2-d8193-fp32.tsv
.../epi_arith_v001_correctness 7 candidate 8193 .../20261002T171334Z/correctness/candidate-r2-d8193-fp32.tsv
```

Each call returned code 1, produced no result TSV, and recorded this exact stderr line:

```text
aclrtSynchronizeStream(correctness) failed: ACL status 507035
```

The retry summary records both rows as `MISSING`. No other shape was invoked during this retry.

## Attribution and Open Cause

D8192 passes for both sides. For each tested width above 8192 with an emitted comparison, the fixed Parent and Candidate both fail the same CPU reference limits. Candidate ratios are mixed relative to Parent, and some maximum errors are identical across versions; these measurements do not establish an H3-only regression. They establish a shared wide-FP32 Parent/Candidate correctness failure against this runner's reference. The current evidence cannot distinguish a pre-existing wide-path behavior problem from a mismatch in runner/reference semantics. D8193 remains unmeasured because both calls fail during ACL stream synchronization, before a result is written.

No additional run, source change, performance measurement, or new Revision is authorized by this record.

## Evidence Paths

Primary matrix:

`/home/data4t2/lelinfeng/cann/server_runs/EPI-ARITH-CHAMPION-W2-X/V001/20261002T163852Z/correctness/summary.tsv`

Primary per-shape TSV and log names:

`.../20261002T163852Z/correctness/{parent,candidate}-r2-d{8192,12288,16384,18416,18417,32768}-fp32.tsv`

`.../20261002T163852Z/logs/correctness-{parent,candidate}-r2-d{8192,12288,16384,18416,18417,32768}-fp32.log`

The primary D8193 log files are `.../20261002T163852Z/logs/correctness-parent-r2-d8193-fp32.log` and `.../20261002T163852Z/logs/correctness-candidate-r2-d8193-fp32.log`; neither corresponding TSV exists.

Narrow retry summary and logs:

`/home/data4t2/lelinfeng/cann/server_runs/EPI-ARITH-CHAMPION-W2-X/V001/20261002T171334Z/correctness/summary.tsv`

`/home/data4t2/lelinfeng/cann/server_runs/EPI-ARITH-CHAMPION-W2-X/V001/20261002T171334Z/logs/correctness-parent-r2-d8193-fp32.log`

`/home/data4t2/lelinfeng/cann/server_runs/EPI-ARITH-CHAMPION-W2-X/V001/20261002T171334Z/logs/correctness-candidate-r2-d8193-fp32.log`

The retry environment snapshot, including pre/post device 7 and process observations, is `.../20261002T171334Z/logs/correctness-supplement-environment.log`. Existing PID 439848 (`python3`) remained present in both snapshots and was not touched. No correctness runner process remained after the two calls.
