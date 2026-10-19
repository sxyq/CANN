# N05 V003 Handoff

V003 final source commit is `bb3a6ddf5ee2f297ae670ed3d06e3476b317b743`, based on V002 commit `59e60d9d424fed045ab6c4e337d5931117c2d337`. The net source delta is limited to the pass-two parameter-tile `col` cursor in `ProcessWideLowPrecision`; the existing V001 paired runner and safe `device_reset=NO` exit path were reused unchanged.

Compile and Correctness passed on d4. The complete paired Local raw sample set is saved locally, but its outliers and high CV make `LOCAL_SCORE=NONE`. V001 and V002 `LOCAL_SCORE=NONE` records remain unchanged.

Next revision: continue on this Route branch with one Scalar address-generation change, commit source first, then use the same CANN 8.5 Compile/Correctness/Local chain. No `.codegraph` files or full build tree belong in the evidence commit.
