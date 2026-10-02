# SMALLMID-DATAFLOW-CHAMPION-X V001 Correctness 结果

- `BUILD_RUN_ID`: `20261002T151744Z-device1`
- `CORRECTNESS_RUN_ID`: `20261002T151744Z-device1-correctness2`
- `RUNNER_COMMIT`: `b051d2d8`
- `CANDIDATE_COMMIT`: `9abea741d4d0b40efdc322da5255c463fede481b`
- `CANDIDATE_SHA256`: `a689e5abc03d2770b277812d9a52ae0a1aaf910952b737525e1722f06bdb340a`
- `EXECUTABLE_SHA256`: `159532deffb1ddaf33792c0b7c296c3bcf7cf2f0cc2ea68fbc0e8de7b68e5208`
- `DEVICE`: server3 device `1`, `Ascend910B3`
- `TOOLKIT`: `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`
- `CORRECTNESS`: `FAIL`; command returned `3`
- `ATTEMPT_1`: `INCOMPLETE`; dynamic loader could not find `libgraph.so`, so main and all kernel cases were skipped. Its original log remains at `/home/data4t2/lelinfeng/cann/local-experiments/SMALLMID-DATAFLOW-CHAMPION-X/V001/a689e5abc03d2770b277812d9a52ae0a1aaf910952b737525e1722f06bdb340a/runs/20261002T151744Z-device1/correctness-device1.log`.
- `ATTEMPT_2_LOG`: `/home/data4t2/lelinfeng/cann/local-experiments/SMALLMID-DATAFLOW-CHAMPION-X/V001/a689e5abc03d2770b277812d9a52ae0a1aaf910952b737525e1722f06bdb340a/runs/20261002T151744Z-device1/correctness-device1-attempt2.log`
- `D=2049`: `FAIL`, BF16, rows `80`, matched ratio `0.56540386`, bad elements `71239`, maximum absolute error `4.26492296`, `atol=0.015625`, `rtol=0.015625`, maximum error limit `1.0`.
- `D=3073` and `D=4095`: not run because the harness stopped after the first failing case.
- `RESOURCE`: HBM usage `96%` before and after; AICore usage `36%` before and after; available disk `628 GiB` before and after. Existing Python and VLLM processes were not changed.
- `FOLLOW_UP`: stop V001 here. No performance measurement, Candidate edit, rebuild, or Online submission was performed.
