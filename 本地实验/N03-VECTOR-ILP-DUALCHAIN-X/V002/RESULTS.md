# N03 VECTOR-ILP-DUALCHAIN-X V002 Results

## Identity

- Candidate source: `本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V002/submission.asc`
- Candidate source SHA256: `689fa9bffb93e1246d5d8dd57e83af0ad9c3e70a5addd4b45b10708aaf5cec01`
- Candidate commit: `5d9a02ea070af8cc4cb76c384500b1241b0eb054`
- Parent commit: `1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f`
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- V002 change: from V001, only the independent BF16 cached-row first-pass `residualLocal`/`xLocal` `ToFloat` emission order is swapped. V001's `gamma`/`bias` order remains unchanged. Arithmetic, address generation, data volume, synchronization, and Tile order are unchanged.
- Device: `cann-server3`, `hwnput3`, device `d0` / runtime device index `0`
- Remote run root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V002/5d9a02ea`

## Evidence

- Adapter CANN 8.5 full-link compile: PASS, configure return code `0`, build return code `0`.
- Paired Runner CANN 8.5 compile: PASS, configure return code `0`, build return code `0`.
- Independent device Correctness: FAIL, return code `3`, case `bf16-cache-params-d6144`.
  - Parent golden: `FAIL`, `failures=58`, `max_abs=0.0078125`, `max_rel=0.0075757578`.
  - Candidate golden: `FAIL`, `failures=58`, `max_abs=0.0078125`, `max_rel=0.0075757578`.
  - Parent/Candidate exact match in the same run: PASS, `mismatches=0`.
- Candidate/Parent paired Local: NOT RUN because the independent golden Correctness failed for both binaries.
- Device/process snapshot: `remote-results/pre-correctness-device.txt` and `remote-results/pre-correctness-processes.txt`. No other process was modified.

## Next action

V002 is a negative correctness result under the current golden oracle. Preserve the raw evidence, resolve the existing Parent golden mismatch before timing, then choose the next independent Vector instruction-emission hypothesis.
