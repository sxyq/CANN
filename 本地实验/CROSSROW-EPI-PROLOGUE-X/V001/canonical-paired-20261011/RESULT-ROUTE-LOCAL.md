# V001 MAIN2-CANONICAL-V1 route-local result

- Revision/source commit: `15aa7c1213a0fd0a02fdf29e5fdebcf8c4428ba2`
- Candidate source SHA-256: `3698c229dfb6a4b336e7ee0506eb1ceaf898c1c37c937c0af7d3eef71fbc685d`
- Candidate executable SHA-256: `67ef7172ef82f0937f9591060a90b96d984315f30edafa22b8e8131a56ec8789`
- Parent: frozen `R31B/V011`, source SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; executable SHA-256 `e5d05ac104c99450904a4dda4417d6d5ecbd50d04412bc129622b51a56e2cf64`
- Runner include SHA-256: `2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7`
- Scorer SHA-256: `f43709695115da53e30e58204f8e7c4716486538397394d7e2cb441f3a1d6d7f`
- Frozen anchor vector SHA-256: `617b5670b0cb2c9fbe22ea80c2180c327eaef47468c6e46488c9fc90ea25ce9f`
- Evidence: this directory; `SHA256SUMS` SHA-256 `0520ea4dfbffc3c4c2c82fe2ea3bb4379ddf4dbaa155be5da3dd443443a08c8d`.
- Build: `PASS`, existing canonical build log `../canonical-build-v001-candidate-include-env-20261011.log` (artifact SHA above).
- Correctness: `PASS 7/7`, RC=0/bad=0 for each case, evidence in `../canonical-correctness-20261011/`.

## Protocol and raw validity

- Cases: C12/C13/C14/C16; each has three interleaved pairs in `P-C/C-P/P-C` order, 24 successful sides total.
- Exact command, after `source /usr/local/Ascend/ascend-toolkit/set_env.sh`: `<parent-or-candidate> 7 <rows> <width> <dtype-code> <prefix> 45 31 1 0 1`.
- Every scored side has RC=0, `bad=0`, 31 device-event samples, blocks=1, gap=0, batch_n=1, and the expected source/runner identity. Raw, stats, RC, pre/post snapshots and identity files are retained.
- A prior V001 C12 Parent attempt without the CANN environment exited RC=127 because `libgraph.so` was unavailable. Its failure files remain preserved; only the successful `attempt2` files are used below.
- `same-binary qualification`: `NOT_RUN`; Parent and Candidate are separately built implementation executables using the same frozen harness include.

## Score

The published pure `scoreVectors` function was applied route-locally to the frozen anchor and four Core candidate medians. Formula: `100 * exp(mean(log(anchor_median_us / candidate_median_us)))`.

| Core | Anchor median us | Parent median us | Candidate medians us (run1/run2/run3) | Candidate median us | Candidate faster vs Parent |
|---|---:|---:|---:|---:|---:|
| C12 | 6.58 | 14.76 | 11.32 / 17.90 / 20.06 | 17.90 | 1/3 |
| C13 | 20.80 | 28.84 | 19.74 / 48.22 / 23.10 | 23.10 | 2/3 |
| C14 | 25.96 | 25.94 | 25.52 / 40.64 / 30.84 | 30.84 | 2/3 |
| C16 | 15.06 | 24.36 | 49.96 / 26.26 / 34.78 | 34.78 | 0/3 |

- `MAIN2-CANONICAL-V1 = 58.935569`; delta vs frozen V011 anchor `-41.064431` points.
- Empirical interval `26.980756–128.736247`; paired Parent diagnostic score `88.248022`.
- `QUALITY=POOR`, `STATUS=LOCAL_NEUTRAL`, `RELIABLE=NO`, `NO_PROMOTION=YES`.
- Device/load: NPU7; all 48 per-side pre/post snapshots retain HBM, AICore and process tables. NPU7 used HBM ranged 24,790–24,793/65,536 MB (free 40,743–40,746 MB). External PID 439848 (`python3`, about 21,260 MB) remained present; measurement was retained under the HBM rule and is not a clean/quiet claim.
