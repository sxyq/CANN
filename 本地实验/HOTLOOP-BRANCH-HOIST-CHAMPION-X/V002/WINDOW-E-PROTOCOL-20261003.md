# HBH-09 V002 — Measurement Recovery Window E Protocol

```text
PREDECLARED_UTC=2026-10-03T07:19:44Z
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V002
HYPOTHESIS=HBH-09
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
RUNNER_SHA256=c4d7b89494ed771c5ce89444c7b5ad4f8fd321c087b1fdb80097ce7ba2c82fb7
DEVICE=7
PRIMARY_SHAPE=BF16_D6144_ROWS80
```

Window A's d7 attempt on this primary shape was noise-blocked (`MAD/median
41.08%`, block drift `45.89%`). Window E retries the exact shape on d7 in a
later time window. The 07:19 probe showed HBM use `24790/65536 MB` (about
40746 MB free), no active shared d7 lease, and AICore/AIVector use `46%/35%`
with a resident user Python process. Per admission rules, AICore activity and
that process are recorded but are not independent blockers; no process will
be stopped, paused, or migrated. Require live free HBM of at least 100 MB and
no shared lease at admission.

Use the exact Parent, Candidate, and existing runner under the d7 formal
flock. Run Parent same-binary qualification first (45 warmups, 31 device-event
samples per block, 2 blocks). Only exact-shape `SAME_BINARY_QUAL=PASS` permits
immediate paired timing under the same lease (45 warmups per side, 31 samples
per side per block, 8 alternating blocks). Preserve every sample and
pre/post load snapshot. No Kernel, runner, source, shape, dtype, or filtering
changes are authorized.
