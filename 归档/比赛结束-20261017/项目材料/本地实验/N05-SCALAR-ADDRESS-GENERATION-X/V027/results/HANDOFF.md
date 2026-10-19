# N05 V027 Handoff

V027 source commit is `56b2940aa688637c0d1260caa8f1ac953ae7f1d4`, based on V026 source commit `1fbb6af47078e92057c0686ec96892e6db903472`. The Scalar reduction scratch row base is now derived from `batchRow * reduceStride`. Parent and Candidate local/remote source SHA256 values match, the remote CMakeCache binds distinct input directories, and the saved Runner binary matches the remote build output.

CANN 8.5 compilation passed. The first Correctness launch returned RC=1 without Runner output because its command omitted the established N05 runtime library path; the empty stdout capture is retained and its stderr was not merged into that file. With the recorded CANN library path and `LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6`, the same binary passed all four Parent/Candidate cases. One paired Local run completed 4 cases x 21 pairs on d2. Paired-delta medians were -0.220, +0.820, -0.260, and -0.620 us; every range spans zero and variation is broad, so `LOCAL_SCORE=NONE`.

d2 HBM was 62855/65536 MB before Correctness, before Local, and after Local. AICore readings were 7%, 0%, and 16%. Existing Python PID 816979 using 59487 MB remained present and was not modified. No reset occurred. Raw build, both Correctness launch records, Local samples, source identity, and device snapshots are retained under `results/`.

Continue with a distinct Scalar address-generation factor; keep arithmetic, tile order, DMA/event order, Core ownership, and Vector emission unchanged.
