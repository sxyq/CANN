# N05 V006 Handoff

V006 source commit is `1e2ba118e90b2d98f2df64d61a914f3a965cc1fb`, based on V005 source commit `1c66a1bd6f2a95d92a89e8fe5f2bed331675b04b`. The only new source change is the pass-one reduction row offset cursor in `ProcessWideLowPrecision`; the existing V001 paired runner and safe `device_reset=NO` exit path were reused unchanged.

Compile and Correctness passed on d4. The complete paired Local raw sample set is saved locally, but shared-load outliers make `LOCAL_SCORE=NONE`. V001 through V005 `LOCAL_SCORE=NONE` records remain unchanged.

FULL_SOURCE_SIMILARITY remains the real Main-audited 0.982-0.999 range because the Champion主体 is shared. No cosmetic source changes were used to lower it; any next revision must remain an executable Scalar address-generation change. Effective-change similarity is left for Main's separate statistic.

Next revision: continue on this Route branch with one executable Scalar address-generation change, commit source first, then use the same CANN 8.5 Compile/Correctness/Local chain. No `.codegraph` files or full build tree belong in the evidence commit.
