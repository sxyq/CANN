# SYNC-BARRIER-ELISION-X V061 Result

- Direct Parent: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Single change: delete only the guarded `WaitFlag<MTE3_V>(outputRelease)` in `ProcessWideFp16BatchedOutputPipelined`. Candidate source SHA256: `4efb391c70b8ef033a3b4675f32b39a282760e55f5222364773ea28d31c15977`.
- Runner labels: rebuilt executable contains `CANDIDATE_V061` and `PARENT_R31B_V011`; no V059 label. Candidate executable SHA256 `db79236f011bfcff8786d48503a6bab38b9acb2c1d615793b94e6efc64b752ec`; correctness runner SHA256 `d064ef681f4b624e2c91b61bccb3dd9e18b790174c1b89b465ebc041903e6da3`.
- Compile: PASS for `sync_barrier_elision_v061` and `sync_barrier_elision_correctness`; full attempt history is in `logs/compile-v061-support-fix.md`. No Candidate source or CMake change was made for the host-include fix.
- Correctness: PASS, all 7 FP16 cases bitwise equal; raw output in `logs/correctness-v061-after-hostinclude.log`.
- Same-binary qualification: 31 samples each. Parent first/second device-time CVs were 0.50559/0.51530 with median drift fraction 0.213650. Candidate CVs were 0.40809/0.36827 with median drift fraction 0.080390. Raw samples are in `logs/qualification-v061-parent.log` and `logs/qualification-v061-candidate.log`.

## Post-run path audit

The source diff places the deletion in `ProcessWideFp16BatchedOutputPipelined`; the current `parent.asc` contains only that function definition and no call site. For the measured FP16 128x128 shape, the runner passes `availableCoreNum=8`, yielding multiple rows per core, and dispatch enters `ProcessSmallLowPrecisionContiguousBatched`. Therefore the seven correctness cases and both Local blocks did not execute the modified function. Compile PASS remains valid for the source, but Correctness PASS does not validate this deletion and the Local measurements do not measure its effect. Preserve both numeric score definitions and all raw data below; verdict is `LOCAL_REJECTED_UNEXERCISED_CHANGE_NOISY`, with no promotion and Local Best unchanged.

## Local

FP16 128x128 only, device 3, two retained 31-pair blocks (62 total). `P` is Parent and `C` is Candidate; paired delta is `C-P` in microseconds. The runner's paired-median score is `-median(C-P)/median(P)*100`. The separately reported median-latency ratio is `(median(P)-median(C))/median(P)*100`; these are distinct statistics.

| Set | P median us | C median us | Median C-P us | Paired-median score | Median-latency ratio | P/C mean us |
|---|---:|---:|---:|---:|---:|---:|
| Block 1, 31 pairs | 18.3800 | 18.1400 | -0.0400 | +0.217628% | +1.305767% | 14.8219 / 15.6277 |
| Block 2, 31 pairs | 15.2600 | 6.6000 | -7.5000 | +49.148100% | +56.749672% | 13.4077 / 11.4445 |
| Pooled, 62 pairs | 16.4900 | 15.7400 | -0.8100 | +4.912068% | +4.548211% | 14.1148 / 13.5361 |

Pooled mean paired delta is -0.5787 us; pooled mean-latency ratio gain is +4.100009%. Pooled sample standard deviations are 6.6032 us (Parent) and 6.5183 us (Candidate), with CVs 0.46782 and 0.48155. The large block-to-block shift (especially Candidate median 18.14 to 6.60 us) and qualification jitter make the pooled positive result non-repeatable evidence. In addition, the changed function was not exercised, so these numeric results do not evaluate the Candidate deletion. Verdict: `LOCAL_REJECTED_UNEXERCISED_CHANGE_NOISY`; Local Best remains exact `R31B-V011`.

Raw timing records are `logs/local-v061-raw.log` and `logs/local-v061-block2-raw.log`. Device/load snapshots before qualification, each Local block, and after capture are in the corresponding `logs/device3-v061-*.log` files. Device 3 showed 3,428/65,536 MB HBM used (62,108 MB free), 0% AICore, and no process in the pre-Local snapshots. The post-capture snapshot was taken at `2026-10-08T12:14:19.675262118Z`; the V061 device-3 assignment is released. Other device load is preserved in the snapshots; no other process was observed on device 3.

This single-shape route-local result is not comparable to the Official 45.16 score. No Online or shared-ledger action was performed.
