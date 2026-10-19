# N05 V001 Handoff

V001 source and paired-runner support are committed locally. All Compile, Correctness, Local raw output, device snapshots, and failed attempts are present under `V001/results`.

The Local result is `LOCAL_SCORE=NONE`: d4 was shared with existing workloads and the paired samples had large outliers. Correctness passed for all four public cases.

Next: create V002 from `V001/submission.asc`, make one Scalar address-generation change in pass two only, commit that source, and continue the existing CANN 8.5 paired Compile/Correctness/Local path on d4. Do not add `.codegraph` files.
