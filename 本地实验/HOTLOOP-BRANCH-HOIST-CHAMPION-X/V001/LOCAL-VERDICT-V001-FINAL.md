# HOTLOOP-BRANCH-HOIST-CHAMPION-X V001 Final Local Verdict

```text
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V001
HYPOTHESIS=HBH-10
DIRECT_PARENT=R31B V011
SHAPE=BF16-D32768
DEVICE=4
FINAL_VERDICT=LOCAL_REJECTED
FORMAL_SCORE_CLAIM=NONE
ONLINE=NOT_PERMITTED
```

## Evidence

Two valid formal windows used the same exact Parent source, Candidate source,
performance executable, device-event timing, warmup count, sample count, and
alternating Parent/Candidate block order.

| Window | Samples | Block median Candidate-vs-Parent deltas | Qualification / load |
|---|---:|---|---|
| 1 | 248 | `-1.267609%, +0.071586%, -0.141244%, -0.636044%` | same-binary MAD/median `1.4524%`; d4 HBM `93%` |
| 2 | 248 | `-0.071280%, 0.000000%, +0.278936%, +0.722542%` | same-binary MAD/median `1.9184%`; d4 HBM `93%` |

Window 1 and Window 2 are directionally mixed. The second window does not
reproduce a stable gain, and its small block deltas are below the observed
same-binary noise scale. No Local score is claimed and no Official score is
inferred.

## Source identity

```text
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=4dc1973ef198cd67693610b6ffedad6883b59f52207b9804052975a13f4e032b
PERFORMANCE_EXECUTABLE_SHA256=dadafae6835854a799f4fc63ca1c5ae45c4495500ed18bfd9ac7f141c0ea9e61
WINDOW_1_RAW_SHA256=274ef5319d9a386711f50e8cca510aac552a22802fa0aee9a7c4f1ba726b6d3f
WINDOW_2_RAW_SHA256=63cc098a4c8832e8a733633e61625c2ce2d488b1688559ad75b376a7ab9219f3
```

The V001 Candidate must now be explicitly restored to the exact R31B V011
source before any sibling is considered. HBH-09 remains forbidden until the
restore commit is complete and the route status is updated.
