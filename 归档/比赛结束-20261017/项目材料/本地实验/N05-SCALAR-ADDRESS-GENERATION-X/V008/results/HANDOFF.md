# N05 V008 Handoff

V008 source commit is `8f2f031e3f2884b663793f91a1782c5daabe53da`, based on V007 source commit `78ff4fa803401fbcbb0e57edb4804a2d79f14816`. The only new source change is the pass-one next-row GM offset cursor in `ProcessWideLowPrecision`; the existing V001 paired runner and safe `device_reset=NO` exit path were reused unchanged.

Compile and Correctness passed on d4. The complete paired Local raw sample set is saved locally, but the remaining outlier makes `LOCAL_SCORE=NONE`. V001 through V007 `LOCAL_SCORE=NONE` records remain unchanged.

FULL_SOURCE_SIMILARITY remains the real Main-audited 0.982-0.999 range because the Champion主体 is shared. No cosmetic source changes were used to lower it; any next revision must remain an executable Scalar address-generation change. Effective-change similarity is left for Main's separate statistic.

Next revision: continue on this Route branch with one executable Scalar address-generation change, commit source first, then use the same CANN 8.5 Compile/Correctness/Local chain. No `.codegraph` files or full build tree belong in the evidence commit.
