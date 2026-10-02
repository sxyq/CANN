# UB-DIAG-2 correctness evidence

- Date: 2026-10-02 UTC
- Host: `hwnput3`
- Worktree: `/home/data4t2/lelinfeng/cann-w2-m2-ub`
- Branch: `w2/m2/ub-lifetime-safe`
- Device: `7` (`Ascend 910B3`, `dav-2201`)
- Toolkit environment: `source /usr/local/Ascend/ascend-toolkit/set_env.sh`
- Protocol executable: `build/ub_diag_2_parent` or `build/ub_diag_2_candidate`, `RUNS=2`
- Protocol: same `diagnostic_runner.cpp`, eight cases, dtype 0 FP32 storage = 4 bytes, dtype 1 FP16 = 2 bytes, dtype 2 BF16 = 2 bytes; timing and Online were not run.
- Raw logs: the four `correctness-*-20261002.log` files in this directory. Each contains pre/post `npu-smi info`, complete stdout/stderr, return code, source identity, first mismatch, actual/reference values, error metrics, NaN/Inf counts, reference `invRms`, tail classification, and repeatability.

## Source and run identity

| source | SHA256 | processes | executable SHA256 | result / return code |
|---|---|---:|---|---:|
| Parent `R31B V011` | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` | 2 × 2 runs | `20fd2b283f3ca4cf3d9f07a32e92fadfc78d49428045473ae1a3ae0856811adf` | `DIAG_COMPLETE FAIL`, `3` |
| Candidate `V001` | `9b73bb5626b5e98be53faeb91eba024b2589bf0b8b5b46e36c48ec18e009f989` | 2 × 2 runs | `9c80858325e904cc463f86e9535f39af72b351f3a04d658697450c37d6e6b06b` | `DIAG_COMPLETE FAIL`, `3` |

The parent and candidate executables were built from their exact byte-bound
sources by the committed UB-DIAG-2 harness. No Candidate Kernel source was
modified during the diagnostic.

## Common PASS cases

Both source identities produced the same result for every run in these seven
cases. `actual/reference` below gives first and last output values. All entries
had `actual_nan=0 actual_inf=0 reference_nan=0 reference_inf=0`, first mismatch
`NONE`, and `DIAG_REPEATABILITY ... IDENTICAL differing_elements=0`.

| case | geometry | tail | max abs | max rel | reference `invRms` min..max | actual first / reference first | actual last / reference last |
|---|---|---|---:|---:|---:|---:|---:|
| `fp16-wide-16384` | 1×16384 | NON_TAIL | 0.001953125 | 0.00179856119 | 6.49436474..6.49436474 | 0.582519531 / 0.582519531 | -0.646972656 / -0.646972656 |
| `fp16-wide-16384-rows3` | 3×16384 | NON_TAIL | 0.001953125 | 0.00224466901 | 6.48541403..6.49436474 | 0.582519531 / 0.582519531 | 1.32324219 / 1.32324219 |
| `fp16-wide-32768` | 1×32768 | NON_TAIL | 0.001953125 | 0.00192678231 | 6.48986006..6.48986006 | 0.58203125 / 0.58203125 | -0.18359375 / -0.18359375 |
| `fp16-wide-32768-rows2` | 2×32768 | NON_TAIL | 0.001953125 | 0.0109890113 | 6.48986006..6.49085093 | 0.58203125 / 0.58203125 | 0.0340881348 / 0.0340881348 |
| `fp16-boundary-8192` | 1×8192 | NON_TAIL | 0.0009765625 | 0.00126182963 | 6.48996925..6.48996925 | 0.58203125 / 0.58203125 | 1.13867188 / 1.13867188 |
| `fp16-tail-16385` | 1×16385 | TAIL (1) | 0.0009765625 | 0.00123609393 | 6.49434376..6.49434376 | 0.582519531 / 0.582519531 | -0.931640625 / -0.931640625 |
| `bf16-wide-16384` | 1×16384 | NON_TAIL | 0.0078125 | 0.00699300691 | 6.49402237..6.49402237 | 0.58203125 / 0.58203125 | -0.6484375 / -0.6484375 |

`actual_inv_rms` is unavailable because the kernel exposes no `invRms` output;
the reference values are nevertheless printed in every raw log.

## Failing FP32 case

The only failing case is `fp32-wide-16384`, geometry 1×16384, NON_TAIL. In all
four processes and all eight FP32 runs, `first_mismatch=index=0 row=0 col=0`;
the reference output is `0.58221519`, reference `invRms` is `6.49427986`, and
all actual/reference NaN and Inf counts are zero.

| source / process | run | failures | actual first / reference first | actual last / reference last | max abs | max rel |
|---|---:|---:|---:|---:|---:|---:|
| Parent / 1 | 1 | 13235 | 0.590929866 / 0.58221519 | -0.639432371 / -0.647432685 | 0.0924866199 | 125.275658 |
| Parent / 1 | 2 | 13236 | 0.585072398 / 0.58221519 | -0.647432327 / -0.647432685 | 0.0924866199 | 63.188633 |
| Parent / 2 | 1 | 13227 | 0.590929866 / 0.58221519 | -0.639432371 / -0.647432685 | 0.0924866199 | 125.275658 |
| Parent / 2 | 2 | 11321 | 0.582214892 / 0.58221519 | -0.647432327 / -0.647432685 | 0.0895314217 | 125.275658 |
| Candidate / 1 | 1 | 15227 | 0.60478735 / 0.58221519 | -0.644527495 / -0.647432685 | 0.0964607 | 63.7908859 |
| Candidate / 1 | 2 | 14255 | 0.590929866 / 0.58221519 | -0.639432371 / -0.647432685 | 0.0924866199 | 125.275658 |
| Candidate / 2 | 1 | 12247 | 0.596072376 / 0.58221519 | -0.647432327 / -0.647432685 | 0.0924866199 | 63.188633 |
| Candidate / 2 | 2 | 12828 | 0.585072398 / 0.58221519 | -0.647432327 / -0.647432685 | 0.0924866199 | 63.188633 |

Run-to-run byte repeatability for this case was `DIFFERENT` in every process:

| source / process | differing elements | first diff index | previous → current first output |
|---|---:|---:|---:|
| Parent / 1 | 12544 | 0 | 0.590929866 → 0.585072398 |
| Parent / 2 | 12032 | 0 | 0.590929866 → 0.582214892 |
| Candidate / 1 | 13312 | 0 | 0.60478735 → 0.590929866 |
| Candidate / 2 | 11392 | 0 | 0.596072376 → 0.585072398 |

## Classification

`FINAL_CLASSIFICATION: PARENT_INSTABILITY`

The corrected FP32-wide protocol makes the failure observable with the exact
Parent source as well as the Candidate source. The Parent fails the same
FP32 case in both independent processes and is non-repeatable within each
process; therefore it is not a stable correctness baseline from which a
Candidate-specific attribution can be made. The Candidate reproduces the same
failure family, while the FP16 retained-y path and unchanged BF16 controls pass
identically. This evidence supports `PARENT_INSTABILITY`, not
`CANDIDATE_BUG`, `SHARED_PATH_SIDE_EFFECT`, `HARNESS_BUG`, or a generic
`UNRESOLVED_WITH_EXACT_BLOCKER`.

The previous runner's FP32 host-width defect was corrected in this harness
(dtype 0 is 4-byte storage/reference). The remaining Parent failure is from
the exact device execution under the corrected protocol, not from that prior
host decode mismatch. No performance Revision, V002, Online action, process
termination, or shared-ledger update was performed.
