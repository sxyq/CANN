# Correctness Run 002 Repeatability Evidence

## Scope and Identity

- Shape/dtype/device: rank-2 `[2,16384]` FP32 on d2 / NPU 2.
- Existing binaries were run in order Parent rep1, Candidate rep1, Parent rep2, Candidate rep2. No binary was rebuilt for these repeats.
- Candidate commit: `fe4f61166c5fd356b216b32d1a8ac55b5d2f3275`.
- Candidate source SHA-256: `7098d7330fa1c29af8928be66c41eb48f0d2310c2d7746c7902c31e58cce25fb`.
- Parent source SHA-256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Parent executable SHA-256: `1e4561b0693c92504884f76a2af3c227f3ccd1cdb6a9bdfa23c231b1f3ac6916`.
- Candidate executable SHA-256: `e48dd8b96f55fda3a9e9481f379627e5ee559dab89a9dbe9e2c4a3621e92fda6`.
- The build log directory is `/home/data4t2/lelinfeng/cann/server_runs/SELECTIVE-FASTPATH-CHAMPION-X/V001-fe4f6116/support/logs/`.

`correctness_parent.asc` defines `SELECTIVE_V001_SOURCE` as `"parent.asc"`. The Parent object's dependency record resolves that include to `/home/data4t2/lelinfeng/cann/server_runs/SELECTIVE-FASTPATH-CHAMPION-X/V001-fe4f6116/support/../parent.asc`. Its SHA-256 is `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`, equal to `/home/data4t2/lelinfeng/cann/线上结果/R31B/V011/submission.sha256`. The Candidate dependency record resolves to the staged `submission.asc` with the Candidate SHA above.

The Candidate selector only chooses STORE V003 for FP32 rank-2 `[8,16384]`, `[1,32768]`, and `[1,16384]`. For `[2,16384]`, the recorded selector result is `add_rms_norm_bias_custom<float>`; each Candidate log reports `path=R31B_V011`, `tileWidth=4096`, `tileCount=4`, and `mergeRowRuns=false`.

## Host Input Evidence

Both executables include the same `correctness_main.inc`. For each process, the host constructs X and residual using `InputValue(i, 37, 11)` and `InputValue(i, 17, 3)`, constructs gamma and bias from fixed formulas, then copies each vector to device memory with `ACL_MEMCPY_HOST_TO_DEVICE`. There is no random or time-based input source. The same source, arguments, host, shape, and dtype therefore generate the same input payload for Parent and Candidate. The `ACL_CHECK` wrapper returns on any failed input copy.

`input-sha256.tsv` was produced once by `repeat_fallback.py`, which mirrors each C++ float32 rounding step and packs little-endian float32 values. These are formula-derived byte digests, not four separately printed runtime measurements. The already-built executables did not log their host-buffer digest at each invocation, and no device buffer readback was added.

| Buffer | Bytes | SHA-256 |
|---|---:|---|
| X | 131072 | `035f7556ac3dcedfa1ddca4a8b3394632436e1298c51181c0add3686e054fbd0` |
| residual | 131072 | `635834ced870019a431e701f9d6d7a02da4ba44b073b0a63fc49a8917f6815a5` |
| gamma | 65536 | `51ece941b3df4a928928d607188f159da7bc1893a2e745fe4a316ea0dbeac91f` |
| bias | 65536 | `9dd869c59a1c5e1526bac7706f0ff4db39f52e8b02a7ab884e616a7aedf88bb2` |
| concatenated X/residual/gamma/bias | 393216 | `442043d61564e446113991769e86411275c34c41f3ae3544e4f08c72c69f6c50` |

## Results

Golden criteria are `atol=2^-16`, `rtol=2^-10`, matched ratio at least `0.99`, and max absolute error at most `1e-2`. All four calls returned `3` after failing the host expected-value comparison. Each output is 131072 bytes.

| Run | RC | max_abs | bad | matched ratio | Output SHA-256 | Full log |
|---|---:|---:|---:|---:|---|---|
| Parent rep1 | 3 | 1.23598542706 | 25578 | 0.219421387 | `a7ffabc199d23bc2ad45c2f643b9078d567f9a6d559e5ce1e02056a84c5b57d3` | `parent_r2_d16384_t0_rep1.log` |
| Candidate rep1 | 3 | 1.23598542706 | 26473 | 0.192108154 | `7ab5115955dc98e6bd1fdfeb9eda1e897b406588f7e5ff2eb2adc110e2923b6d` | `candidate_r2_d16384_t0_rep1.log` |
| Parent rep2 | 3 | 1.27598538892 | 23667 | 0.277740479 | `5fbef010b1df6537604c5d729333add354048aebca5cedbed1e9002703321042` | `parent_r2_d16384_t0_rep2.log` |
| Candidate rep2 | 3 | 1.23464994934 | 20723 | 0.367584229 | `465dd2286b2e9cae1dfac063e43db8d2cca11ef21b13b1704edebe1f2e35cc2f` | `candidate_r2_d16384_t0_rep2.log` |

The Parent output SHA-256 differs between rep1 and rep2. The Candidate output SHA-256 also differs between rep1 and rep2. Classification: `PARENT_BASELINE_INSTABILITY / CANDIDATE_COMPARISON_UNRESOLVED`. No specific output-error mechanism is inferred.

All original files remain at `/home/data4t2/lelinfeng/cann/server_runs/SELECTIVE-FASTPATH-CHAMPION-X/V001-fe4f6116/support/logs/correctness-run-002-repro/`, including `input-sha256.tsv`, `repeat-summary.tsv`, `manifest.json`, four output binaries, four TSV results, and the four logs transcribed below.

## Complete Logs

### Parent rep1

```text
COMMAND=/home/data4t2/lelinfeng/cann/server_runs/SELECTIVE-FASTPATH-CHAMPION-X/V001-fe4f6116/support/build/selective_v001_parent_correctness 2 2 16384 0 /home/data4t2/lelinfeng/cann/server_runs/SELECTIVE-FASTPATH-CHAMPION-X/V001-fe4f6116/support/logs/correctness-run-002-repro/parent_r2_d16384_t0_rep1.bin /home/data4t2/lelinfeng/cann/server_runs/SELECTIVE-FASTPATH-CHAMPION-X/V001-fe4f6116/support/logs/correctness-run-002-repro/parent_r2_d16384_t0_rep1.tsv
--- stdout ---
variant=parent rank=2 rows=2 width=16384 dtype=FP32 path=R31B_V011 tileWidth=4096 tileCount=4 mergeRowRuns=false
golden_max_abs=1.23598542706 golden_bad=25578 matched_ratio=0.219421387 max_abs_limit=0.01 status=FAIL
--- stderr ---
RC=3
```

### Candidate rep1

```text
COMMAND=/home/data4t2/lelinfeng/cann/server_runs/SELECTIVE-FASTPATH-CHAMPION-X/V001-fe4f6116/support/build/selective_v001_candidate_correctness 2 2 16384 0 /home/data4t2/lelinfeng/cann/server_runs/SELECTIVE-FASTPATH-CHAMPION-X/V001-fe4f6116/support/logs/correctness-run-002-repro/candidate_r2_d16384_t0_rep1.bin /home/data4t2/lelinfeng/cann/server_runs/SELECTIVE-FASTPATH-CHAMPION-X/V001-fe4f6116/support/logs/correctness-run-002-repro/candidate_r2_d16384_t0_rep1.tsv
--- stdout ---
variant=candidate rank=2 rows=2 width=16384 dtype=FP32 path=R31B_V011 tileWidth=4096 tileCount=4 mergeRowRuns=false
golden_max_abs=1.23598542706 golden_bad=26473 matched_ratio=0.192108154 max_abs_limit=0.01 status=FAIL
--- stderr ---
RC=3
```

### Parent rep2

```text
COMMAND=/home/data4t2/lelinfeng/cann/server_runs/SELECTIVE-FASTPATH-CHAMPION-X/V001-fe4f6116/support/build/selective_v001_parent_correctness 2 2 16384 0 /home/data4t2/lelinfeng/cann/server_runs/SELECTIVE-FASTPATH-CHAMPION-X/V001-fe4f6116/support/logs/correctness-run-002-repro/parent_r2_d16384_t0_rep2.bin /home/data4t2/lelinfeng/cann/server_runs/SELECTIVE-FASTPATH-CHAMPION-X/V001-fe4f6116/support/logs/correctness-run-002-repro/parent_r2_d16384_t0_rep2.tsv
--- stdout ---
variant=parent rank=2 rows=2 width=16384 dtype=FP32 path=R31B_V011 tileWidth=4096 tileCount=4 mergeRowRuns=false
golden_max_abs=1.27598538892 golden_bad=23667 matched_ratio=0.277740479 max_abs_limit=0.01 status=FAIL
--- stderr ---
RC=3
```

### Candidate rep2

```text
COMMAND=/home/data4t2/lelinfeng/cann/server_runs/SELECTIVE-FASTPATH-CHAMPION-X/V001-fe4f6116/support/build/selective_v001_candidate_correctness 2 2 16384 0 /home/data4t2/lelinfeng/cann/server_runs/SELECTIVE-FASTPATH-CHAMPION-X/V001-fe4f6116/support/logs/correctness-run-002-repro/candidate_r2_d16384_t0_rep2.bin /home/data4t2/lelinfeng/cann/server_runs/SELECTIVE-FASTPATH-CHAMPION-X/V001-fe4f6116/support/logs/correctness-run-002-repro/candidate_r2_d16384_t0_rep2.tsv
--- stdout ---
variant=candidate rank=2 rows=2 width=16384 dtype=FP32 path=R31B_V011 tileWidth=4096 tileCount=4 mergeRowRuns=false
golden_max_abs=1.23464994934 golden_bad=20723 matched_ratio=0.367584229 max_abs_limit=0.01 status=FAIL
--- stderr ---
RC=3
```

No timing, additional device run, Candidate edit, or shared-record edit was made for this evidence follow-up.
