# V003 MAIN2-CANONICAL-V1 route-local result

- Route/revision: `CROSSROW-EPI-PROLOGUE-X / V003`
- Source commit: `5347a5606cdc8aec4c350c7d6e0b987b94346a8c`
- Candidate source SHA-256: `c6d8da88a9ba03e6c730733c52bed051b54ebc94629196636ea415599b96eb8c`
- Parent: frozen `R31B/V011`, source SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate executable SHA-256: `59e6580388840ebdff36ebf66d1f1809abaab4d11b173f8a300252e86c878340`
- Parent executable SHA-256: `e5d05ac104c99450904a4dda4417d6d5ecbd50d04412bc129622b51a56e2cf64`
- Shared runner include SHA-256: `2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7`
- Scorer SHA-256: `f43709695115da53e30e58204f8e7c4716486538397394d7e2cb441f3a1d6d7f`
- Frozen anchor vector SHA-256: `617b5670b0cb2c9fbe22ea80c2180c327eaef47468c6e46488c9fc90ea25ce9f`
- Data: this directory; checksum manifest `SHA256SUMS` SHA-256 `a60f78c5bdd0ef5a5a3b2385068849a8daaeef7a2dce48e0ad68c60d7a2cdc44`.
- Build: `PASS` for Candidate and V011 Parent; logs `../canonical-build-v003-candidate-20261011.log` and `../canonical-build-v011-parent-20261011.log`.
- Correctness: `PASS 7/7`, RC=0/bad=0 for each case; evidence in `../canonical-correctness-20261011/`.

## Protocol and validation

- Canonical suite: seven correctness cases passed `7/7`, `bad=0`, RC=0 (see `../canonical-correctness-20261011/`). The four scored Core cases were C12/C13/C14/C16.
- Each Core has three 31-sample device-event runs per side, with 45 warmups, blocks=1, gap=0, batch_n=1; interleave order verified from RC timestamps: P-C, C-P, P-C.
- Validated all 24 raw TSVs and all 24 RC files through the published `validateRun`/`summarizeRuns` functions. Each raw has 31 ordered samples and matches case shape, device 7, event method, source identity, and runner include SHA. `SHA256SUMS` covers the saved paired evidence. Parent and Candidate use separate implementation executables but the same frozen runner include; separate same-binary qualification: `NOT_RUN`.
- Exact command form (for each side/case/run): `<parent-or-candidate-executable> 7 <rows> <width> <dtype-code> <raw-prefix> 45 31 1 0 1`; raw and stats are emitted by that existing executable. No new runner or scorer was created; no shared scoreboard command was run.

## Frozen-score result

The published pure `scoreVectors` function was applied route-locally to the frozen anchor vector and the validated raw summaries. Formula: `100 * exp(mean(log(anchor_median_us / candidate_median_us)))` over the four Core cases.

| Core | Anchor median (us) | Paired Parent median (us) | Candidate median (us) | Candidate runs faster than paired Parent |
|---|---:|---:|---:|---:|
| C12 | 6.58 | 18.56 | 10.98 | 2/3 |
| C13 | 20.80 | 30.66 | 24.92 | 1/3 |
| C14 | 25.96 | 66.26 | 30.60 | 2/3 |
| C16 | 15.06 | 25.58 | 19.38 | 2/3 |

- `MAIN2-CANONICAL-V1 = 75.778905`; delta vs frozen V011 anchor `-24.221095` points.
- Empirical interval: `28.710045–200.015099`; paired Parent diagnostic score `156.141781`.
- Quality `POOR`; status `LOCAL_NEUTRAL`; reliable gain `NO`; promotion `NO`.
- Window context: device 7 showed 24,790/65,536 MB HBM in the captured full snapshot (40,746 MB free), AICore 38%, and external PID 439848 (`python3`, 21,260 MB). Load was retained as context; it is consistent with the large run-to-run dispersion and is not represented as a clean/quiet window.

The earlier Candidate-only files in `../canonical-local-20261011/` are preserved and excluded from this score. V003 source and binaries were not changed during scoring.
