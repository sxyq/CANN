# N05 V002 Handoff

V002 source commit `59e60d9d424fed045ab6c4e337d5931117c2d337` is based on V001 commit `47c59a77156799f9f58f83e1bcfd1bb8013309f`. The only source change is the pass-two Store row-offset cursor in `ProcessWideLowPrecision`; the existing V001 paired runner and its safe `device_reset=NO` exit path were reused unchanged.

Compile and Correctness passed on d4. The complete paired Local raw sample set is saved locally, but its outliers and CV greater than 1 make `LOCAL_SCORE=NONE`. V001's `LOCAL_SCORE=NONE` is retained.

Next revision: continue on this Route branch with one Scalar address-generation change, commit source first, then use the same CANN 8.5 Compile/Correctness/Local chain. No `.codegraph` files or full build tree belong in the evidence commit.
