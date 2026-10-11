# N04 V002 Handoff

V002 Candidate source is committed locally as `46ee7f3d` and its SHA256 is recorded in `RESULTS.md`. The only Kernel change from V001 is the output GM L2 hint in `ProcessWideLowPrecision`; the V001 input hint remains in place.

The d1 preflight is saved. No remote N04 process is running. Compile, Correctness, and Local evidence do not exist yet and must be produced with the existing CMake and paired Runner under `~/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V002/46ee7f3d`. Put the system libstdc++ directory first in the Runner environment as used for V001, and preserve all raw output and return codes.
