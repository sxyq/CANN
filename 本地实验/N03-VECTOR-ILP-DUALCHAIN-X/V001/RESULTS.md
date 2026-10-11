# N03 VECTOR-ILP-DUALCHAIN-X V001 Results

## Identity

- Candidate source: `本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V001/submission.asc`
- Candidate source SHA256: `39a807bdf4d5b5732bb18c58cf4b8983a43fcaf18540ee0431b2045a8a56377d`
- Candidate kernel commit: `9be20982612bbc3a4f409731a88cbee4098e48ca`
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Parent commit: `1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f`
- Kernel change: BF16 cached-parameter path swaps the emission order of the independent `gamma` and `bias` `ToFloat` operations. Arithmetic, address generation, data volume, synchronization, and tile order are unchanged.
- Device: `cann-server3`, `hwnput3`, device `d0` / runtime device index `0`
- Remote run root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V001/9be20982612bbc3a4f409731a88cbee4098e48ca`

## Evidence

- Adapter CANN 8.5 compile: PASS, return code `0`.
- Paired Runner CANN 8.5 rebuild after the golden-check support change: PASS, return code `0`.
- Parent/Candidate exact compare from `remote-results/correctness-exactcompare.raw.log`: PASS, `mismatches=0`. This run only checked equality between the two outputs.
- Independent golden Correctness from `remote-results/candidate-correctness-systemlib.raw.log`: FAIL, return code `3`.
  - Parent golden: `FAIL`, `failures=58`, `max_abs=0.0078125`, `max_rel=0.0075757578`.
  - Candidate golden: `FAIL`, `failures=58`, `max_abs=0.0078125`, `max_rel=0.0075757578`.
  - Parent/Candidate exact match: PASS, `mismatches=0`.
- Runtime setup attempts are preserved: `candidate-correctness.raw.log` records the initial loader failure, `candidate-correctness-envfix.raw.log` records the CANN environment-only loader failure, and `candidate-correctness-systemlib.raw.log` is the actual device run.
- Same-binary Parent qualification is preserved in `remote-results/same-binary-control.raw.log`: 62 samples, median `10.440 us`, MAD/median `0.08429`, block drift `0.01916`, reported `PASS`. This is a control measurement, not a Candidate Local Score.
- Candidate/Parent paired Local: NOT RUN because the independent golden Correctness failed for both binaries.
- Device/process snapshot: `remote-results/pre-candidate-correctness-device.txt` and `remote-results/pre-candidate-correctness-processes.txt`. No other process was modified.

## Next action

V001 is a negative correctness result for the current golden oracle. Preserve the evidence, diagnose the existing Parent golden mismatch before using timing as evidence, and select the next independent Vector-ILP single-point hypothesis only after the correctness path is resolved.
