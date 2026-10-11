# V002 MAIN2-CANONICAL-V1 route-local result

- Revision/source commit: `4d3f3c234d9eeae7f1bc93ee250272f6fa54b70d`
- Candidate source SHA-256: `426489bfdc63441f2fd179fdff176875b24d815e6e0420343aeed0b0dab7933f`
- Candidate executable SHA-256: `b57ba4e0370ce8b9615eafba2b934d5fbeab446fe29b3ef5f6846809e8f138ee`
- Parent: frozen `R31B/V011`, source SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; executable SHA-256 `e5d05ac104c99450904a4dda4417d6d5ecbd50d04412bc129622b51a56e2cf64`
- Runner include SHA-256: `2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7`
- Scorer SHA-256: `f43709695115da53e30e58204f8e7c4716486538397394d7e2cb441f3a1d6d7f`
- Frozen anchor vector SHA-256: `617b5670b0cb2c9fbe22ea80c2180c327eaef47468c6e46488c9fc90ea25ce9f`
- Evidence: this directory; `SHA256SUMS` SHA-256 `0ae2e88f94e38e343fae6a8a16ccc74783e9e35beb1687b1ac9439f3b8c37b3a`.
- Build: `PASS`, existing canonical build log `../canonical-build-v002-candidate-20261011.log` (artifact SHA above).
- Correctness: `PASS 7/7`, RC=0/bad=0 for each case, evidence in `../canonical-correctness-20261011/`.

## Protocol and raw validity

- Cases: C12/C13/C14/C16; each has three interleaved pairs in `P-C/C-P/P-C` order, 24 successful sides total.
- Exact command, after `source /usr/local/Ascend/ascend-toolkit/set_env.sh`: `<parent-or-candidate> 7 <rows> <width> <dtype-code> <prefix> 45 31 1 0 1`.
- Every scored side has RC=0, `bad=0`, 31 device-event samples, blocks=1, gap=0, batch_n=1, and the expected source/runner identity. Raw, stats, RC, pre/post snapshots and identity files are retained.
- `same-binary qualification`: `NOT_RUN`; Parent and Candidate are separately built implementation executables using the same frozen harness include.

## Score

The published pure `scoreVectors` function was applied route-locally to the frozen anchor and four Core candidate medians. Formula: `100 * exp(mean(log(anchor_median_us / candidate_median_us)))`.

| Core | Anchor median us | Parent median us | Candidate medians us (run1/run2/run3) | Candidate median us | Candidate faster vs Parent |
|---|---:|---:|---:|---:|---:|
| C12 | 6.58 | 18.56 | 17.86 / 17.86 / 15.50 | 17.86 | 2/3 |
| C13 | 20.80 | 21.10 | 19.48 / 20.04 / 27.48 | 20.04 | 2/3 |
| C14 | 25.96 | 63.30 | 25.56 / 69.96 / 25.56 | 25.56 | 2/3 |
| C16 | 15.06 | 23.24 | 21.58 / 16.82 / 24.28 | 21.58 | 2/3 |

- `MAIN2-CANONICAL-V1 = 72.153409`; delta vs frozen V011 anchor `-27.846591` points.
- Empirical interval `32.296987–161.195053`; paired Parent diagnostic score `130.700485`.
- `QUALITY=POOR`, `STATUS=LOCAL_NEUTRAL`, `RELIABLE=NO`, `NO_PROMOTION=YES`.
- Device/load: NPU7; all 48 per-side pre/post snapshots retain HBM, AICore and process tables. NPU7 used HBM ranged 24,773–24,793/65,536 MB (free 40,743–40,763 MB). External PID 439848 (`python3`, about 21,260 MB) remained present; measurement was retained under the HBM rule and is not a clean/quiet claim.
