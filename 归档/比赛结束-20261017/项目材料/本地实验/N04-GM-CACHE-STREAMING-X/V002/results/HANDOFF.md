# N04 V002 Handoff

V002 is the V001 input L2 disable hint plus one output-path L2 disable hint. Candidate source commit `46ee7f3d3cba1c781c3ce68a8dfdd7251a641247` has SHA256 `dcbc9bfbb710daa6829922dac2a762e84cbc54b32f30d64ac6397c056a2dc86e`. The exact source and dependency upload is recorded in `upload-verification.txt`.

Compile completed after the existing paired CMake entry was pointed at the current remote source directory. Correctness passed 4/4. Paired Local completed 21 pairs for each of the four public cases with 45 warmups; raw output is in `local-paired.log`. Because d1 carried sustained unrelated load and the deltas had outliers, the result is `LOCAL_SCORE=NONE`.

The remote run directory is `~/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V002/46ee7f3d`. No N04 process remains. V003 should use V002 as Parent and make exactly one new output cache-mode choice on the same GM/L2 axis.
