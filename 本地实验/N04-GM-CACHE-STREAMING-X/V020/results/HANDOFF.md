# N04 V020 Handoff

Candidate commit `e2a5b36cf`, source SHA-256 `d17d17335443581bcdb64cb8c1625f8d2f1b1e9295c5cabea95b8d726e3ed5c4`. Relative to direct parent V019 commit `1c80adf17`, it changes only the wide low-precision `biasGm_` hint from `PERSISTENT` to `NORMAL`.

The existing CANN 8.5 `paired_runner_v016` entry compiled successfully. Paired Champion SHA-256 is `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; executable SHA-256 is `bd47685567583fc9f46c2e07fc4b6c8d4ad3bffa470b0b0eaacca76e079f4b4f`. Correctness passed 4/4 on d7 with return code 0.

Local was not run. The earlier pre-Local d7 reading showed AICore 6% and AIVector 15%, with shared PID 439848 using 21,260 MiB. A fresh reading at `2026-10-12T07:37:10+00:00` later showed d7 idle (AICore/AIVector 0%, HBM 5%, no device process), but `/home/data4t2` had only 2.0 MB free at 100% use. The existing Same-binary qualification and paired Local were therefore not started, and no samples were collected. `LOCAL_SCORE=NONE`. Snapshot: `pre-local-recheck-d7.txt`. No Judge request was made.

Latest read-only resume snapshot (`2026-10-12T11:50:28+00:00`) records `/home/data4t2` at 100% displayed capacity with 6,815,252 KiB available. NPU d7 reported AICore 19% and HBM 60,613/65,536 MiB; Python PID 3954164 used 57,243 MiB on that device. The process was left untouched, and V020 Local/profile were not started. Snapshot: `pre-local-resume-20261012T115028Z.txt`.

The existing V020 paired object contains an embedded `.aicore_binary`, but no device instruction listing or msprof evidence is saved for this route. Main's verified Official comparison is N04 V017 41.85 vs R31B V011 45.16; testcase dtype/Rows/D remain UNKNOWN, so no Official Case is assigned to a Local shape. No evidence currently selects a new cache Hint factor. Resume V020 Local and collect route-level profile evidence when d7 and the output volume permit it; use that evidence before changing another source factor. Do not submit this route to Judge; a qualified identity belongs with N06.

On 2026-10-12 at 12:36:34 UTC, a fresh read-only snapshot showed d7 AICore 7%, HBM 60,614/65,536 MiB, and Python PID 3954164 using 57,243 MiB. `/home/data4t2` had 6,815,756 KiB available while reporting 100% capacity. Same-binary P/P and Local were not run because shared compute remained active; no samples were collected and no process was touched. Snapshot: `pre-local-decision-20261012T123634Z.txt`.

The existing V019 and V020 executables each pair their own version with R31B V011; neither can directly interleave V019 against V020. A direct pair would require a new combined Runner build, which was not attempted. V020's earlier compile and 4/4 Correctness were not repeated. `LOCAL_SCORE=NONE`; retain V020 unchanged. No Judge submission was made.

## 2026-10-13 Direct V019/V020 Pair Preparation

- The existing V020 `paired_runner_v016` source/configuration is adapted in the local worktree to use N04 V019 as Parent and N04 V020 as Candidate. The target name remains `paired_runner_v016`.
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n04-gm-cache-streaming`; branch `route/n04-gm-cache-streaming-x-20261011`; preparation started from HEAD `fe602519704545657839ba0b1cbcb0216a22e270`.
- Kernel inputs were verified locally and by read-only remote SHA-256:
  - V019: `/home/data4t2/lelinfeng/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V019/1c80adf17/source/N04-V019-GM-CACHE-STREAMING-X_kernel.asc` = `13435a1b4b14e5094f99931cfe99647a4cf43603bea8592cf580743870cbcaf5`.
  - V020: `/home/data4t2/lelinfeng/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V020/e2a5b36cf/source/N04-V020-GM-CACHE-STREAMING-X_kernel.asc` = `d17d17335443581bcdb64cb8c1625f8d2f1b1e9295c5cabea95b8d726e3ed5c4`.
- Adapter identities: `paired_runner_v016.asc` = `a85b8c00c50c706f0363e35599743edc5845404663ffe7833f0e962d7b4e5aa4`; `paired_runner.cpp` = `778c0c28bc124c6022090bb09f260855639aa143b2a2b2e11c85b074f124f502`; `paired-CMakeLists.txt` = `5098708912a558d564b54f5a01a91075af947eba1c33973d796cff798f60e346`; ABI header = `096c0229f5d9e6fa37460808387517280cb0f2e4296e07b8c37f75cd73c89617`.
- The ASC source directly includes both Kernel sources; CMake adds both source directories. The existing V019 paired executable is not an input dependency. Its SHA-256 is `d3de11ce4967e47175dbc14a47e3596d2bded44d998cb1c22e91e9fe8e4776ce`. The existing V020/R31B paired executable remains at `results/artifacts/paired_runner_v016` with SHA-256 `bd47685567583fc9f46c2e07fc4b6c8d4ad3bffa470b0b0eaacca76e079f4b4f`; it was not replaced.
- `TEST_NOT_RUN`: Correctness, P/P, Local, and Profile were not run, and the adapted target was not built. The latest user-provided d7 state reports AICore 8% and shared Python PID 3954164. No device process was started or touched. Server3 was queried read-only; the N04 Git branch update is recorded by the push.
- Recovery point: continue from the three adapted files under `本地实验/N04-GM-CACHE-STREAMING-X/V020/results/`, using existing target `paired_runner_v016`. These files were not copied to server3; a future build must copy them into the existing `/home/data4t2/lelinfeng/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V020/e2a5b36cf/support/paired/` directory first. The earlier V020/R31B logs and executable remain unchanged.
