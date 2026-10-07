# W4-R11 V002 RESULT

Parent: R31B V011. The sole performance change assigns `extraRows` to the highest-index blocks; block count, row arithmetic, tile choices, and active-core count remain unchanged.

## Build and correctness

- server3 build: PASS. The measurement runner was rebuilt after fixing its `same`-mode raw stream argument; Candidate performance source was not changed in this turn.
- Device: Ascend 910B3, device 0; runtime reported 40 vector cores.
- Target `64x8192 FP32`: Parent and Candidate correctness both PASS; maximum absolute error `4.76837e-06` for each.
- Control `12x8192 FP32`: Parent and Candidate correctness both PASS; maximum absolute error `3.09944e-06` for each.
- The target exercises `blockCount=40`, `extraRows=24`; the control has `blockCount=12`, `extraRows=0`. These are local proxy shapes, not an asserted Official testcase mapping.

## Parent same-binary qualification

Each shape has 82 Parent samples across two blocks. Qualification did not pass the existing stability limits:

| Shape | Median (us) | MAD/median | Block medians (us) | Block drift | Result |
|---|---:|---:|---:|---:|---|
| 64x8192 FP32 | 21.19 | 0.158565 | 21.62 / 20.72 | 0.0424729 | MEASUREMENT_BLOCKED |
| 12x8192 FP32 | 19.92 | 0.146084 | 19.04 / 22.34 | 0.165663 | MEASUREMENT_BLOCKED |

The earlier same-mode attempts are retained. Their first sample exposed a null raw-stream argument in the runner; the direct runner fix now passes the opened stream to same mode. This is a measurement-tool fix and does not alter Candidate performance logic.

## Interleaved Parent/Candidate Local observations

The runner collected 44 Parent/Candidate pairs per shape, alternating order. Numeric paired median deltas are retained as observations; because Parent same-binary qualification failed, they are not accepted as a Local Best.

| Shape | Parent median (us) | Candidate median (us) | Median paired delta | Parent MAD/median | Candidate MAD/median |
|---|---:|---:|---:|---:|---:|
| 64x8192 FP32 | 9.85 | 9.72 | -2.76053% | 0.0517766 | 0.0462964 |
| 12x8192 FP32 | 8.32 | 7.07 | -1.83724% | 0.241587 | 0.154173 |

The control has substantial dispersion. Keep the per-shape numeric deltas and all raw samples, but do not promote V002. `CURRENT_LOCAL_BEST=NONE`; `OFFICIAL_SCORE=NONE`; Online remains PAUSED.

## Device context and evidence

- Admission: device 0 had at least 62,108 MB free by the full NPU snapshot; the usages readout reported 5% HBM use on a 65,536 MB device. The required 100 MB threshold was met.
- Device 0 AICore use was 0% in the pre/post snapshots. Other NPU devices had active processes; none were stopped or changed.
- Host load averages were about 44.65 before and 38.96 after the Local run. Record as measurement context only.
- Same-binary stdout/raw: `logs/same-binary-device0-debug7.stdout.txt`, `results/same-binary-device0-debug7.raw.tsv`.
- Interleaved Local stdout/raw: `logs/local-device0-pc-after-debug7.stdout.txt`, `results/local-device0-pc-after-debug7.raw.tsv`.
- Admission snapshots: `logs/local-device0-preflight.txt`, `logs/local-device0-postflight.txt`.
- Runner-fix compile transcript: `logs/server3-compile-runner-fix.log`.
- The same-mode crash raw header and the earlier failed-run evidence remain alongside these files.
