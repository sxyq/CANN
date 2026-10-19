# N05 V025 Handoff

V025 source commit is 57601e93369371893f33e69148f0e13a30c953a9, based on V024 source commit f2ad29fdcbe1d030c0aeeb66eddab50f198162fd. Pass-two output GM row addresses are now derived directly from the batch base, row, and tile column instead of advancing a Store cursor. Parent and Candidate local/remote SHA256 values match and CMakeCache records separate source directories.

CANN 8.5 compilation passed. Correctness passed all four cases for Parent and Candidate. One paired Local run completed 4 cases x 21 pairs on d2. Paired-delta medians were +0.420, -1.560, +2.780, and +0.440 us. Ranges span zero and the measurements vary widely, so LOCAL_SCORE=NONE.

Before Local, d2 HBM was 62855/65536 MB and AICore usage was 26%. Existing Python PID 816979 remained present and was not modified. No reset occurred. Raw build, Correctness, Local, identity, and device snapshots are retained under results/.

Continue with a distinct Scalar address-generation factor; do not change Vector emission or operation order.
