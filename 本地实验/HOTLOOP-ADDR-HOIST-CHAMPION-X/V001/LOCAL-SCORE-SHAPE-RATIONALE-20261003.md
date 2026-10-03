# ADDR H3 V001 Local-score shape rationale

```text
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
REVISION=V001
HYPOTHESIS=ADDR-H3 stateful pass-1 index counters
DIRECT_PARENT=R31B V011
PRIMARY_LOCAL_SCORE_SHAPE=BF16-D32768-ROWS9
TARGET_DTYPE=BF16
DEVICE=7
```

BF16-D40960 remains a separately recorded shared runtime blocker: the proven
Parent and the H3 Candidate both fail at `aclrtSynchronizeStream=507035`.
It is not used for timing.

The selected fallback is a legal H3 wide-low-precision path that exercises the
reduced-tile address regime (`tileWidth=2560`, `tileCount=13`, `tail=2048`)
and the same flattened pass-1 row/tile counter state as H3.  The exact H3
correctness runner already recorded Parent PASS, Candidate PASS, and exact
output equality for this shape on device 7 in:

```text
logs/correctness-run-fix1-local-hwnput3-d7-20261002.txt
```

The timing runner is the existing known-good paired runner, with only its
device and shape constants changed.  No Candidate Kernel, mathematical
reference, or timing algorithm was changed.
