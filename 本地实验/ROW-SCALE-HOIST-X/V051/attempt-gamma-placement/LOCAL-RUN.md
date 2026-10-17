# V051 Local Run

- Runner build: `/tmp/cann-row-scale-hoist-x-v051-runners-retry.XDwor6`; all four correctness/local targets built. The first runner build failure (`ASCEND_HOME_PATH` unset) and the retry configure log are preserved separately.
- Candidate source identity: `5aad6db8fb2be25d65cd39c42c6a5ce72b75fd6e485b4495905c0c1d46f25bba`; Local Candidate executable SHA-256 `4ee792dcbfa355075e065eb2be3df2a7b53694464ca62d3d0851b4f7ddaecdb0`. Parent source identity is exact V026 SHA `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Device 0, exclusively assigned to this Route through numeric capture. Pre-Local snapshot at `2026-10-08T05:36:59Z`: 12,367/65,536 MB used, 53,169 MB free. Existing Python process was left undisturbed.
- Each process performs 20 warmups and captures 32 ACL device-event samples. Four P/C blocks ran serially in interleaved order: P1, C1, P2, C2, P3, C3, P4, C4. All eight processes exited 0 and reported correctness PASS and the intended dispatch; Candidate reports the source delta executed.
- Raw samples are retained in `local-parent-block1.log`, `local-candidate-block1.log`, ..., `local-parent-block4.log`, `local-candidate-block4.log`; no samples were excluded.
- Pooled medians are Parent `19.6799995 us`, Candidate `19.21 us`; medians descriptively favor Candidate by `2.388208902139%`. Pooled means are Parent `20.34640621875 us`, Candidate `23.784218578125 us`, making Candidate `16.896410709656%` slower by mean. Candidate sample CV is `1.2718951206`, with a maximum sample of `289.679993 us`; three of four paired block medians are slower.
- Verdict: `MEASUREMENT_BLOCKED`, not reliable acceptance/rejection. Current Local Best remains V026.
- Device 0 explicitly released after raw capture at `2026-10-08T05:38:02Z`; final device/process evidence is in `device-release.log`.
