# N05 V023 Handoff

V023 source commit is d5e070374e9f21890326916c5b07cf687a5959ce, based on V022 source commit a05054d177c849c4132e0ffd6671298e26453298. It removes the pass-one input GM tile cursor and derives the prefetched offset from row and tile coordinates. Parent and Candidate local/remote source SHA256 values match; their remote directories are separate.

CANN 8.5 compilation passed after two retained environment failures. The successful attempt used ASCEND_HOME_PATH and the established GCC 11 CPLUS_INCLUDE_PATH. Correctness passed 4/4 after one retained loader-path failure. The one paired Local run completed 4 cases x 21 pairs on d2. Paired-delta medians were -4.060, +0.620, +0.360, and +3.360 us; every range spans zero and variation is broad, so LOCAL_SCORE=NONE.

Device snapshots show d2 HBM at 62855/65536 MB before both Correctness and Local. Existing Python PID 816979 remained present and was not modified. No reset occurred. Raw compiler, correctness, Local, identity, and device evidence remains under results/.

Continue with a different Scalar address-generation factor; preserve the operation sequence and do not change Vector emission.
