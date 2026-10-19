# N05 V007 Handoff

V007 source commit is `78ff4fa803401fbcbb0e57edb4804a2d79f14816`, based on V006 source commit `1e2ba118e90b2d98f2df64d61a914f3a965cc1fb`. The only new source change is the pass-one batch-row reduction offset cursor in `ProcessWideLowPrecision`; the existing V001 paired runner and safe `device_reset=NO` exit path were reused unchanged.

Compile and Correctness passed on d4. The complete paired Local raw sample set is saved locally, but shared-load outliers make `LOCAL_SCORE=NONE`. V001 through V006 `LOCAL_SCORE=NONE` records remain unchanged.

FULL_SOURCE_SIMILARITY remains the real Main-audited 0.982-0.999 range because the Champion主体 is shared. No cosmetic source changes were used to lower it; any next revision must remain an executable Scalar address-generation change. Effective-change similarity is left for Main's separate statistic.

Next revision: continue on this Route branch with one executable Scalar address-generation change, commit source first, then use the same CANN 8.5 Compile/Correctness/Local chain. No `.codegraph` files or full build tree belong in the evidence commit.
