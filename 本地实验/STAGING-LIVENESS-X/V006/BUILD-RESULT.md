ROUTE: STAGING-LIVENESS-X
REVISION: V006
DIRECT_PARENT: STAGING-LIVENESS-X/V005
PARENT_SOURCE_SHA: be0e8f794c135b5d5a0544be1b574dd874446fe9067da35c82e2435454211366
CANDIDATE_SOURCE_SHA: 627c0fdf94412e1b1546c1bbd6f3cb370f8040b7a1f0c3207e41511605e1cac3

## Compile record

- Final result: COMPILE PASS at `2026-10-06T19:11:22.734717645Z`.
- Scope: local CMake `device` and `submission` object targets; `full_link` was not run.
- Command: `cmake -S 本地实验/STAGING-LIVENESS-X/V006 -B 本地实验/STAGING-LIVENESS-X/V006/build`, then `cmake --build 本地实验/STAGING-LIVENESS-X/V006/build --target device submission -j2`.
- The successful compile used the exact edited source; root and `src/submission.asc` SHA-256 values match.
- Correctness: NOT RUN. Local performance: SUSPENDED_FOR_W4. Online: FORBIDDEN.
