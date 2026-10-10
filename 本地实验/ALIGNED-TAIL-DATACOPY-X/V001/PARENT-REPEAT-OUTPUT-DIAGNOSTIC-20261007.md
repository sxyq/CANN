# Parent Repeat-Output Diagnostic

- Route: `ALIGNED-TAIL-DATACOPY-X`
- Classification: `PARENT_HARNESS_OR_BASELINE_BLOCKED`
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` (the unchanged `submission.asc`, identical to R31B V011)
- Scope: read-only comparison of preserved full-suite logs; no runner or NPU command was started for this diagnostic.
- Compared runs:
  - `parent-correctness-full-final-20261007.log` (F)
  - `parent-correctness-resume-20261007.log` (R)
  - `parent-correctness-repeat-valuediag-20261007.log` (D)

## Finding

All three preserved full-suite runs report 66 cases and the same six FP32-wide failures; each therefore has 60 passing cases. The adjacent FP32 `4x8192` case passes in F and R. The `RUN_META` records device 7, 40 vector cores, and epsilon `9.99999975e-06`; F and R both record `launch_cores=40`.

For every failing shape, the logged row-0/column-0 `x`, residual, gamma, bias, and golden values are identical in F, R, and D. The per-element `allowed` value at that coordinate is also identical across runs. The first actual value and aggregate output signature vary:

| Shape | Golden[0] | allowed[0] | Actual[0] F / R / D | Matched ratio F / R / D | Max abs F / R / D |
|---|---:|---:|---|---|---|
| FP32 4x8193 | 1.67460977 | 0.00165061989 | 1.15396905 / 1.15396905 / 2.10932302 | 0.410502868 / 0.371811302 / 0.640424753 | 2.04067187 / 2.04067187 / 1.90147054 |
| FP32 2x16383 | -1.15705378 | 0.00114519412 | -2.45548344 / -2.63624978 / -0.976287603 | 0.281847037 / 0.430537753 / 0.188884820 | 2.20100126 / 2.17746293 / 2.16654224 |
| FP32 2x16384 | 1.26097774 | 0.00124668237 | 1.57180190 / 1.70697856 / 0.762867928 | 0.239349365 / 0.219787598 / 0.219757080 | 2.18135605 / 2.18135605 / 2.17315027 |
| FP32 2x16385 | 0.807561072 | 0.000803892648 | 0.735062897 / 0.735062897 / 0.563858569 | 0.216081782 / 0.278730546 / 0.219774184 | 2.19517557 / 2.26609103 / 2.19517557 |
| FP32 1x32768 | 0.949928801 | 0.000942923634 | 0.345996559 / 0.583672404 / 0.448312104 | 0.127014160 / 0.064392090 / 0.126495361 | 2.14984641 / 2.07876313 / 2.13813484 |
| FP32 1x32769 | 0.238674710 | 0.000248339560 | 0.363988042 / 0.285087347 / 0.302523673 | 0.094693155 / 0.125881168 / 0.126186335 | 2.28426506 / 2.28426506 / 2.28426506 |

The sampled row-0/column-0 inputs (`x`, residual, gamma, bias) and golden are respectively:

| Shape | x | residual | gamma | bias |
|---|---:|---:|---:|---:|
| FP32 4x8193 | 1.83061218 | 0.525552154 | 0.841673195 | 0.141344756 |
| FP32 2x16383 | -1.86590767 | -0.335825145 | 0.600539207 | -0.126130283 |
| FP32 2x16384 | 1.61953497 | 0.382071137 | 0.947318196 | -0.216340244 |
| FP32 2x16385 | 0.0428805351 | 0.745854497 | 1.24355030 | 0.0491976738 |
| FP32 1x32768 | 0.177213669 | 0.898843050 | 1.38687110 | -0.204578742 |
| FP32 1x32769 | -0.746303320 | 0.877141237 | 0.800472736 | 0.157049000 |

## Interpretation and Limit

This is evidence of nondeterministic Parent-path outputs for repeated logged inputs, not evidence of an input, golden, or logged tolerance mismatch. The mismatch magnitudes are far beyond the unchanged `allowed` thresholds. The records do not include full input/golden/output tensor hashes, an RNG seed, or a complete runner identity and synchronization/buffer-lifetime trace. They therefore cannot distinguish a kernel-side race/undefined behavior from a runner-side launch, stream, or buffer-lifetime discrepancy. No concrete root cause or minimal support fix is established.

## Gate and Planning Evidence Needed

- Keep the unchanged Parent Correctness gate failed; retain `PARENT_HARNESS_OR_BASELINE_BLOCKED`.
- No Candidate edit/V001, Local timing, or further speculative runner/harness probes are authorized by this evidence.
- Planning needs to provide either Parent-valid-case evidence for these exact shapes and source SHA, or an authoritative Parent runner package that identifies its executable/source, exact inputs and goldens (seed or full-buffer hashes), tolerance formula, tiling/dispatch and launch metadata, and stream synchronization/output-buffer lifetime contract.
- After that evidence is available, resume only with a direct comparison against these preserved logs; further execution requires a concrete, testable discrepancy and an authorized minimal support fix.

No original Parent source, Candidate, runner/harness, launch setting, shared record, or prior log was modified.
