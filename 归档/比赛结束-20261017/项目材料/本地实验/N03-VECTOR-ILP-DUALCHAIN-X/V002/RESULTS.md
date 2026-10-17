# N03 VECTOR-ILP-DUALCHAIN-X V002 Results

## Identity

- Candidate source: `本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V002/submission.asc`
- Candidate source SHA256: `689fa9bffb93e1246d5d8dd57e83af0ad9c3e70a5addd4b45b10708aaf5cec01`
- Candidate commit: `5d9a02ea070af8cc4cb76c384500b1241b0eb054`
- Parent source: R31B V011 Champion
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Parent commit: `1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f`
- V002 change: only the independent BF16 cached-row first-pass `residualLocal`/`xLocal` `ToFloat` emission order is swapped from V001. V001's `gamma`/`bias` order remains unchanged. Arithmetic, address generation, data volume, synchronization, and Tile order are unchanged.
- Device: `cann-server3`, host `hwnput3`, d0 / runtime device index `0`
- Remote run root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V002/5d9a02ea`
- Correctness support: stable double-precision RMS reference and BF16 mixed tolerance from `3ff6771e`

## Compile

- Adapter CANN 8.5 full-link compile: PASS, configure and build return code `0`.
- Paired Runner CANN 8.5 compile: PASS, configure and build return code `0`.
- V002 paired CMake binding: `R31B_V011_SOURCE_DIR` points to the remote `parent` directory and `R31B_V016_SOURCE_DIR` points to the remote `candidate` directory. No V003 input was used.
- Device artifacts: `device.alink` SHA256 `9ba79108a38d6dc7887bfa729e3f2aaa3bb492b576914598fdb572381f408561`; `submission.alink` SHA256 `59af1da96359c0016988dc279d0a96b3f471f2fa1874a3a62d06f388a5520551`.
- Paired Runner executable SHA256: `a09dd9cb30ace1cd09569e6bb3ca12067f593c5c8bc0154f89d2b9d6a293946b`; device object SHA256: `32cb25c987bb56eab1a3920b9cf35a6a87ab7aa9552a5b7c87a4e538d862876c`.

## Correctness

- Old `0.001` comparison is preserved in `results/remote-results/candidate-correctness.raw.log` and `golden-diagnostic.raw.log`: Parent and Candidate each had `58/98304` failed elements, `max_abs=0.0078125`, `max_rel=0.0075757578`; Parent/Candidate exact match had `0` mismatches. This was the Runner reference/tolerance issue.
- Corrected BF16 mixed-tolerance run: PASS, return code `0`.
- Case: `bf16-cache-params-d6144`, dtype BF16, shape `[16,6144]`, elements `98304`.
- Parent golden: `0/98304` failures, matched ratio `1.000000000`, `max_abs=0`, `max_rel=0`.
- Candidate golden: `0/98304` failures, matched ratio `1.000000000`, `max_abs=0`, `max_rel=0`.
- Parent/Candidate exact match: PASS, `mismatches=0`, `max_abs=0`.
- Mixed tolerance: `rtol=0.015625`, `atol=0.015625`, `max_abs_limit=1`.
- Corrected evidence: `results/remote-results/v002-current-correctness.raw.log` and `correctness-mixed-tolerance.raw.log`.

## V002 Control And Paired Rerun

Run id: `rerun-v002-20261011T092525Z`. The same CANN 8.5 binary was used for both modes after source binding was recorded.

### Parent Same-Binary Control

- Command: `results/remote-results/rerun-20261011T092525Z/rerun-v002-20261011T092525Z-same-binary-control.command`
- Case: `bf16-cache-params-d6144`, Parent V011, `62` device-event samples in two blocks of `31`.
- Correctness: PASS, `mismatches=0`, `max_abs=0`.
- Parent median: `10.510 us`; mean `28.025 us`; stdev `80.770 us`; MAD `0.850 us`; p10 `9.246 us`; p90 `16.294 us`; max `574.160 us`.
- Same-binary quality result: PASS (`mad_over_median=0.08088`, `block_drift=0.05519`), with large individual outliers retained in the raw log.

### V011 Parent / V002 Candidate Paired Run

- Command: `results/remote-results/rerun-20261011T092525Z/rerun-v002-20261011T092525Z-paired.command`
- Case: `bf16-cache-params-d6144`, BF16 `[16,6144]`, warmup `45` each, `21/21` complete pairs, `21` samples per binary.
- Correctness in paired run: PASS, Parent/Candidate output match `0` mismatches and `max_abs=0`.
- Parent median: `10.200 us`; Candidate V002 median: `11.460 us`; paired Candidate minus Parent median delta: `+1.220 us`.
- Parent raw range: `8.260–28.920 us`; Candidate raw range: `8.960–258.860 us`.
- Paired delta range: `-16.520–248.140 us`; paired delta p10 `-2.040 us`, p90 `7.740 us`.
- Sample validity: `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`, `REPEATABILITY=UNCONFIRMED`; the Candidate `258.860 us` outlier and paired `+248.140 us` delta prevent a performance conclusion. Raw samples are retained without filtering.
- Raw sample file: `results/remote-results/rerun-20261011T092525Z/rerun-v002-20261011T092525Z-paired.raw.log`.

The earlier V002 paired run is also retained in `results/remote-results/v002-paired-local.raw.log`: `21/21` pairs, Parent median `10.560 us`, Candidate median `11.440 us`, delta `+0.440 us`, with Candidate `440.760 us` and Parent `272.440 us` outliers. It also remains `LOCAL_SCORE=NONE`.

## Device And Process Evidence

- Rerun identity and source/artifact SHA evidence: `results/remote-results/rerun-20261011T092525Z/rerun-v002-20261011T092525Z-identity.txt`.
- Pre/post device snapshots: `rerun-v002-20261011T092525Z-pre-control-device.txt`, `post-control-device.txt`, `pre-paired-device.txt`, `post-paired-device.txt`.
- Pre/post process snapshots: matching `*-processes.txt` files in the same directory.
- d0 HBM capacity was `65536 MB`, HBM usage rate was `91%` in the four rerun snapshots, leaving about `5.7 GB` free; AICore usage was `31%`, `27%`, `26%`, and `30%` at the four points. Other processes were present and recorded; none was stopped or changed.
- The first setup attempt is retained under `results/remote-results/rerun-20261011T091841Z/`; it stopped before device execution because `set -u` made the CANN environment script reject an unset `LD_LIBRARY_PATH`. It did not produce a measurement and is not included in the score.
- No Route Runner process remained after the rerun.

## Status

- V002 BF16 mixed-tolerance Correctness: PASS, both Parent and Candidate `0/98304` failures.
- V002 Parent control: PASS, `62` samples.
- V002 paired Local rerun: completed, `21/21` pairs, but invalid for a performance claim because of the retained outliers and concurrent device load.
- No V002 Candidate source or performance code was changed during this rerun. No V003 input was used or modified by this round.
- No new Revision was created.
