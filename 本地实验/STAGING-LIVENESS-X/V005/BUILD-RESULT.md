ROUTE: STAGING-LIVENESS-X
REVISION: V005
DIRECT_PARENT: STAGING-LIVENESS-X/V004
PARENT_SOURCE_SHA: 320fd709b248900475d53e63a4ea1e1993415dbc2c1a365bb41442049d9b2962
CANDIDATE_SOURCE_SHA: be0e8f794c135b5d5a0544be1b574dd874446fe9067da35c82e2435454211366

## Compile record

- Final result: COMPILE PASS at `2026-10-06T18:53:02.991919477Z`.
- Scope: CMake `device` and `submission` object targets; `full_link` was not run.
- Build fix: materialized the exact V005 `submission.asc` under `src/submission.asc`, where `adapter.cpp` includes it. Root and `src` candidate SHA-256 values match.
- The preceding submission compile failure due to the missing materialized source is retained in `server_runs/STAGING-LIVENESS-X/V005-submission-compile.log`.
- Successful device log: `server_runs/STAGING-LIVENESS-X/V005/build/device-compile.log`.
- Successful submission log: `server_runs/STAGING-LIVENESS-X/V005/build/submission-compile.log`.
- Correctness: NOT RUN. Local performance: SUSPENDED_FOR_W4. Online: FORBIDDEN.
