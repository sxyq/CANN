# N05 V026 Handoff

V026 source commit is 1fbb6af47078e92057c0686ec96892e6db903472, based on V025 source commit 57601e93369371893f33e69148f0e13a30c953a9. Pass-two tile columns are derived from tile index and tileWidth instead of an advancing col cursor. Parent and Candidate local/remote SHA256 values match; CMakeCache points to separate input directories.

CANN 8.5 compilation passed. Correctness passed all four cases for Parent and Candidate. One paired Local run completed 4 cases x 21 pairs on d2. Paired-delta medians were +1.940, -0.100, -0.280, and +0.120 us. All ranges span zero and variation is broad, so LOCAL_SCORE=NONE.

d2 snapshots show HBM 62855/65536 MB and AICore 7% before both Correctness and Local. Existing Python PID 816979 remained present and was not modified. No reset occurred. Raw build, Correctness, Local, identity, and device snapshots are retained under results/.

Continue with a distinct Scalar address-generation factor; do not change Vector emission or operation order.
