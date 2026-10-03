# HBH-09 V002 — Measurement Recovery Window B Protocol

```text
PREDECLARED_UTC=2026-10-03T06:40:48Z
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V002
HYPOTHESIS=HBH-09
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
DEVICE=4
PRIMARY_SHAPE=FP32_D6144_ROWS80
```

This is a support-only measurement protocol choice. The Candidate Kernel,
parent, runner binary, shape semantics, and dtype semantics remain unchanged.
The target is a declared HBH-09 primary shape and is selected before reading
Window B timing output.

## Admission snapshot

At predeclaration, d4 reported HBM `61977/65536 MB` (approximately 3559 MB
free), AICore 2%, AIVector 4%, and no active d4 lease in the shared
`调度/服务器设备使用.tsv` state log. Host load was `79.92/72.75/66.18` with
34 users. The device also had resident Python/VLLM processes. These load facts
are observational; no process will be stopped, paused, or migrated. The
measurement is admitted only while live HBM still has at least 100 MB free,
the shared d4 lease state remains conflict-free, and the local d4 flock is
acquired.

## Fixed sequence

1. Capture a fresh all-device, HBM, host-load, process, and lease snapshot.
2. Under `/tmp/hbh09-v002-npu4.lock`, run Parent same-binary on d4 with the
   existing performance runner: 45 warmups, 31 device-event samples per
   block, 2 blocks. No Candidate timing is allowed unless the exact shape
   returns `SAME_BINARY_QUAL=PASS`.
3. On PASS, without releasing the d4 lease or changing either source, run the
   exact Parent/Candidate pair on d4 with 45 warmups per side, 31 samples per
   side per block, and 8 alternating blocks. The runner already supports this
   block count; no harness source or behavior changes are made.
4. Preserve every raw sample and the complete pre/post load snapshots. No
   samples may be removed, trimmed, or selected by result direction.

Eight paired blocks are predeclared because Window A's four-block paired
windows showed elevated Parent jitter and mixed block directions. This only
increases observation count; it does not change the timing boundary or
Parent/Candidate treatment. A same-binary failure ends this device/shape
attempt without P/C. No new performance Revision or Online submission is
authorized by this protocol.
