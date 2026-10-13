# N04 V020 Results

## Candidate

- Route: `N04-GM-CACHE-STREAMING-X`.
- Direct parent: N04 V019, commit `1c80adf17`, source SHA-256 `13435a1b4b14e5094f99931cfe99647a4cf43603bea8592cf580743870cbcaf5`.
- Candidate commit: `e2a5b36cf`.
- Candidate source: `本地实验/N04-GM-CACHE-STREAMING-X/V020/submission.asc`.
- Candidate SHA-256: `d17d17335443581bcdb64cb8c1625f8d2f1b1e9295c5cabea95b8d726e3ed5c4`.
- Single change from V019: wide low-precision `biasGm_` hint changes from `CACHE_MODE_PERSISTENT` to `CACHE_MODE_NORMAL`. Gamma, x, residual, and output hints are unchanged. Arithmetic, Vector instructions, Scalar indexing, tile traversal, DMA descriptors, and parameter residency are unchanged.

## Compile

- Host: `hwnput3` via `cann-server3`; CANN `8.5.0.alpha002`; SoC `Ascend910B3`.
- Paired comparison Parent: R31B V011 Champion, SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Existing `paired_runner_v016` CMake configure and Compile passed. Configure and build logs: `configure.log`, `paired-build.log`.
- CMake paths and remote SHA-256 identities: `build-identity.txt`, `remote-inputs.txt`.
- Executable SHA-256: `bd47685567583fc9f46c2e07fc4b6c8d4ad3bffa470b0b0eaacca76e079f4b4f`; saved artifact: `artifacts/paired_runner_v016`.
- `/home/data4t2` had about 6.5 GiB available at the recorded read. No unrelated files were removed or modified.

## Correctness

- PASS, all four FP16/BF16 tail and wide cases for R31B V011 Champion and N04 V020 on d7; Runner return code 0.
- Raw output: `correctness-v011-v020.log`; it ends with `RUNNER_COMPLETE mode=CORRECTNESS correctness=PASS cases=4 device=7` and `REMOTE_EXIT_CODE=0`.
- Immediately before launch at `2026-10-12T02:25:35+00:00`, d7 reported HBM use 37%, AICore 0%, AIVector 0%, HBM bandwidth 11%, and shared Python PID 439848 using 21,260 MiB. The after-run read at `02:30:02+00:00` showed AICore/AIVector/HBM bandwidth at 0%; the same process remained.

## Paired Local

- Status: NOT RUN; `LOCAL_SCORE=NONE`.
- The fresh pre-Local snapshot at `2026-10-12T02:31:59+00:00` showed AICore 6%, AIVector 15%, HBM use 37%, and shared Python PID 439848 using 21,260 MiB. This active shared compute was left untouched; no timed samples were collected.
- A later live recheck at `2026-10-12T07:37:10+00:00` showed d7 idle (AICore 0%, AIVector 0%, HBM use 5%, no process in device), but `/home/data4t2` had only 2.0 MB free at 100% use. The Same-binary qualification and paired Local were not started because the remote experiment filesystem had insufficient free space for new experiment data. No samples were collected; the live snapshot is `pre-local-recheck-d7.txt`.
- Reassess only with a new d7 reading before any later Local run. Reuse the existing Runner with 45 warmups and 21 alternating pairs per case when shared activity is suitable.

## Evidence

- Initial server and d7 state: `pre-resume-server3-d7.txt`.
- Configure/build and inputs: `configure.log`, `paired-build.log`, `build-identity.txt`, `remote-inputs.txt`.
- Correctness and device state: `pre-correctness-d7.txt`, `pre-correctness-d7-recheck1.txt`, `pre-correctness-d7-launch.txt`, `correctness-v011-v020.log`, `post-correctness-d7.txt`.
- Local decisions: `pre-local-d7.txt`, `pre-local-recheck-d7.txt`.
- Existing Runner sources and artifact: `paired_runner_v016.asc`, `paired_runner.cpp`, `paired_runner_abi.h`, `paired-CMakeLists.txt`, `artifacts/paired_runner_v016`.

## Resume Snapshot And Object Review

- Read-only server snapshot at `2026-10-12T11:50:28+00:00` is retained in `pre-local-resume-20261012T115028Z.txt`. `/home/data4t2` reported 6,815,252 KiB available while displaying 100% capacity. NPU d7 reported AICore 19%, HBM 60,613/65,536 MiB, and an existing Python process (PID 3954164) using 57,243 MiB. No process was changed.
- The existing V020 `paired_runner_v016.asc.o` is an AArch64 relocatable ELF (274,152 bytes) containing an `.aicore_binary` section. This confirms an embedded device image, but the available object metadata does not provide instruction-level timing or identify an L2/HBM bottleneck. The N04 evidence tree has no saved msprof collection or device-instruction disassembly.
- No Local or profiler run was started: d7 had active shared compute and the existing Python process remained present. No samples were collected. The six-receipt Official mapping in Main records dtype/Rows/D as UNKNOWN; no Official Case is mapped to a Local shape.

## 2026-10-12 Local Resume Decision

- Fresh read-only snapshot at `2026-10-12T12:36:34+00:00` is recorded in `pre-local-decision-20261012T123634Z.txt`. `/home/data4t2` showed 6,815,756 KiB available at 100% displayed capacity. d7 showed AICore 7%, HBM 60,614/65,536 MiB, and Python PID 3954164 using 57,243 MiB. No process was touched, and no repeated polling was done.
- Same-binary P/P and paired timing were not started because the target had active shared compute and a large resident Python workload; timing under that snapshot would not give a dependable comparison. `LOCAL_SCORE=NONE`; zero samples collected.
- The existing V019 executable pairs R31B V011 with N04 V019; the existing V020 executable pairs R31B V011 with N04 V020. V020 `--qualify-parent` measures R31B V011 alone. Neither existing binary provides interleaved V019/V020 samples. Direct V019/V020 pairing would require a new combined Runner build, so none was attempted. The V019 and V020 source SHA-256 values are `13435a1b4b14e5094f99931cfe99647a4cf43603bea8592cf580743870cbcaf5` and `d17d17335443581bcdb64cb8c1625f8d2f1b1e9295c5cabea95b8d726e3ed5c4`.
- No Correctness was repeated. The existing V020 compile and 4/4 Correctness result remain valid for its recorded source identity.

## 2026-10-13 Direct V019/V020 Pair Preparation

- Purpose: adapt the existing `paired_runner_v016` target to interleave direct Parent N04 V019 and Candidate N04 V020 using their source-level entry points. Candidate source is unchanged; no second Runner was added.
- Worktree and starting point: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n04-gm-cache-streaming`, branch `route/n04-gm-cache-streaming-x-20261011`, starting HEAD `fe602519704545657839ba0b1cbcb0216a22e270`.
- Remote Kernel inputs were read-only verified against local source identities:

| Input | Remote path | SHA-256 |
|---|---|---|
| N04 V019 Kernel source | `/home/data4t2/lelinfeng/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V019/1c80adf17/source/N04-V019-GM-CACHE-STREAMING-X_kernel.asc` | `13435a1b4b14e5094f99931cfe99647a4cf43603bea8592cf580743870cbcaf5` |
| N04 V020 Kernel source | `/home/data4t2/lelinfeng/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V020/e2a5b36cf/source/N04-V020-GM-CACHE-STREAMING-X_kernel.asc` | `d17d17335443581bcdb64cb8c1625f8d2f1b1e9295c5cabea95b8d726e3ed5c4` |
| V019 paired ABI header | `/home/data4t2/lelinfeng/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V019/1c80adf17/support/paired/paired_runner_abi.h` | `096c0229f5d9e6fa37460808387517280cb0f2e4296e07b8c37f75cd73c89617` |
| V020 paired ABI header | `/home/data4t2/lelinfeng/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V020/e2a5b36cf/support/paired/paired_runner_abi.h` | `096c0229f5d9e6fa37460808387517280cb0f2e4296e07b8c37f75cd73c89617` |

- Adapter file identities: `paired_runner_v016.asc` = `a85b8c00c50c706f0363e35599743edc5845404663ffe7833f0e962d7b4e5aa4`; `paired_runner.cpp` = `778c0c28bc124c6022090bb09f260855639aa143b2a2b2e11c85b074f124f502`; `paired-CMakeLists.txt` = `5098708912a558d564b54f5a01a91075af947eba1c33973d796cff798f60e346`.
- `paired_runner_v016.asc` includes both Kernel sources under separate namespaces; `paired_runner.cpp` selects V019 for Parent and V020 for Candidate; CMake includes both remote source directories and keeps target `paired_runner_v016`.
- The existing V019 executable is not required by this source build because both Kernel sources are compiled into the paired target. Its verified SHA-256 is `d3de11ce4967e47175dbc14a47e3596d2bded44d998cb1c22e91e9fe8e4776ce`; it remains untouched.
- The existing V020/R31B executable SHA-256 remains `bd47685567583fc9f46c2e07fc4b6c8d4ad3bffa470b0b0eaacca76e079f4b4f`. It and all prior R31B/V020 result files were preserved.
- `TEST_NOT_RUN`: Correctness, P/P, Local, and Profile were not run; the adapted target was not built. The latest user-provided d7 state is AICore 8% with shared Python PID 3954164. No device task was launched. Server3 access was limited to read-only source, ABI, and binary identity queries; the N04 Git branch was pushed.
- Recovery point: this worktree's `本地实验/N04-GM-CACHE-STREAMING-X/V020/results/paired_runner_v016.asc`, `paired_runner.cpp`, and `paired-CMakeLists.txt`, target `paired_runner_v016`. These files were not copied to server3; a future build must copy them into the existing `/home/data4t2/lelinfeng/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V020/e2a5b36cf/support/paired/` directory first. Starting branch and HEAD were `route/n04-gm-cache-streaming-x-20261011` and `fe602519704545657839ba0b1cbcb0216a22e270`.
