# ROW-OCCUPANCY-CHAMPION-X V001 baseline restore trail

ROUTE=ROW-OCCUPANCY-CHAMPION-X
REVISION=V001
LOCAL_VERDICT=LOCAL_REJECTED
RESTORE_KIND=EXPLICIT_BASELINE_RESTORE_FOR_NEXT_SIBLING
DIRECT_PARENT=R31B/V011
PARENT_SOURCE_PATH=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_PATH=本地实验/ROW-OCCUPANCY-CHAMPION-X/V001/submission.asc
CANDIDATE_SOURCE_SHA256=7757274fe51507a2f8bf599c5771a9f1e9b21dce68c66a0b2b97934d7f4d1315

V001 is retained as an immutable rejected experiment. The next occupancy
sibling must be materialized from the exact R31B/V011 source above, not from
the V001 candidate. The parent digest is verified against the canonical
R31B/V011 source and the candidate digest is retained in V001/source-meta.json
and V001/submission.asc.

This is an explicit restore/reset trail, not a destructive Git reset: no
candidate evidence is deleted, overwritten, or silently replaced. The route
branch contains the frozen parent source under `线上结果/R31B/V011/` and the
rejected candidate under this V001 directory. A future V002 declaration must
repeat the parent SHA and record its own sibling source SHA before build.

RESTORE_VERIFICATION=PASS
NEXT_SIBLING_PARENT=R31B/V011
NEXT_SIBLING_PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
