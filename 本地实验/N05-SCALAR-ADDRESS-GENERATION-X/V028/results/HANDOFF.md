# N05 V028 Handoff

V028 source commit is `d6f9606dbc500e2fb0e1d17dd4603a729f3bec38`, based on V027 source commit `56b2940aa688637c0d1260caa8f1ac953ae7f1d4`. Pass-one retained-y UB tile addresses are derived from `batchRow * rowWidth + col`; this differs from the V022 pass-two address change. Parent and Candidate local/remote SHA256 values match, CMakeCache points to distinct input directories, and the saved Runner binary matches the remote build output.

CANN 8.5 configuration and compilation passed. Correctness passed all four cases for Parent and Candidate. One paired Local run completed 4 cases x 21 pairs on d2. Paired-delta medians were -1.860, +0.900, +0.100, and +0.640 us. Every range spans zero, and the BF16 D=32768 range contains a -52.200 us outlier; `LOCAL_SCORE=NONE`.

d2 HBM readings were 62855/65536 MB before Correctness, 62855/65536 MB before Local, and 62856/65536 MB after Local. AICore readings were 13%, 11%, and 14%. Existing Python PID 816979 using 59487 MB remained present and was not modified. No reset occurred. Build, Correctness, Local, identity, and device snapshots are retained under `results/`.

The current window ends with V028. Do not create or run V029 in this window.
