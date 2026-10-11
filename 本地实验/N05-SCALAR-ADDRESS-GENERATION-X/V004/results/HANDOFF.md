# N05 V004 Handoff

V004 source commit is `9f0bd92f1dff81987aacfff6dd1447b1688e8b7c`, based on V003 source commit `bb3a6ddf5ee2f297ae670ed3d06e3476b317b743`. The only new source change is the pass-two local value-row offset cursor in `ProcessWideLowPrecision`; the existing V001 paired runner and safe `device_reset=NO` exit path were reused unchanged.

Compile and Correctness passed on d4. The complete paired Local raw sample set is saved locally, but shared-load outliers make `LOCAL_SCORE=NONE`. V001, V002, and V003 `LOCAL_SCORE=NONE` records remain unchanged.

Next revision: continue on this Route branch with one Scalar address-generation change, commit source first, then use the same CANN 8.5 Compile/Correctness/Local chain. No `.codegraph` files or full build tree belong in the evidence commit.
