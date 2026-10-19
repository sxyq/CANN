# N05 V024 Handoff

V024 source commit is f2ad29fdcbe1d030c0aeeb66eddab50f198162fd, based on V023 source commit d5e070374e9f21890326916c5b07cf687a5959ce. The scalar reduction pass now derives each reduction-scratch row address from batchRow and reduceStride instead of advancing a row cursor. Parent and Candidate sources were independently placed on server3 and their local/remote SHA256 values match.

CANN 8.5 compilation passed with the existing toolchain initialization and GCC 11 paths in both CPLUS_INCLUDE_PATH and CPATH. Correctness passed 4/4. One paired Local run completed 4 cases x 21 pairs on d2. Paired-delta medians were -0.340, +0.800, +0.360, and +1.020 us. Every range spans zero, and one case has a -1649.460 us extreme delta; LOCAL_SCORE=NONE.

d2 snapshots show HBM 62855/65536 MB before both device runs, AICore 23% before Correctness and 18% before Local. Existing Python PID 816979 remained present and was not modified. No reset occurred. Raw build, Correctness, Local, identity, and device evidence remains under results/.

Continue with a distinct Scalar address-generation factor; do not change Vector emission or operation order.
