# N03 V002 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- HEAD before evidence update: `7c3fe809dd3da794d7c7344b2d342ba3c219e6d7`
- Candidate commit: `5d9a02ea070af8cc4cb76c384500b1241b0eb054`
- Candidate source SHA256: `689fa9bffb93e1246d5d8dd57e83af0ad9c3e70a5addd4b45b10708aaf5cec01`
- Parent: R31B V011, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Device: `cann-server3` / `hwnput3` / d0
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V002/5d9a02ea`

## Final V002 Evidence

- CANN 8.5 Adapter and paired Runner compile: PASS, return code `0`.
- Corrected BF16 Correctness: PASS for Parent and Candidate, `0/98304` failures each, exact match `0` mismatches, `max_abs=0`, `max_rel=0`, `rtol=atol=0.015625`.
- Parent same-binary control: PASS, `62` samples, median `10.510 us`.
- V011 Parent / V002 Candidate paired run: PASS at the output-comparison level, `21/21` pairs. Parent median `10.200 us`, Candidate median `11.460 us`, delta `+1.220 us`.
- Measurement validity: `LOCAL_SCORE=NONE`. Candidate max device time was `258.860 us`, paired delta max was `+248.140 us`, and d0 had HBM usage `91%` with AICore usage `26–31%`; full raw samples and snapshots are retained.

## Evidence Paths

- Correctness: `results/remote-results/v002-current-correctness.raw.log`, `correctness-mixed-tolerance.raw.log`, and preserved old `candidate-correctness.raw.log` / `golden-diagnostic.raw.log`.
- Rerun raw and command files: `results/remote-results/rerun-20261011T092525Z/`.
- Rerun identity and artifact SHA: `rerun-v002-20261011T092525Z-identity.txt`.
- Rerun device/process snapshots: matching `pre-*` and `post-*` files in that directory.

## Scope

- This handoff is only for fixed V002. No V003 or later source change was made for this round, and existing V003 files remain untouched.
- No new Revision was created. No Local score improvement is claimed.
