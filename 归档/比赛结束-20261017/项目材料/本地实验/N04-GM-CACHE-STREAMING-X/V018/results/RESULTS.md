# N04 V018 Results

## Candidate

- Route: `N04-GM-CACHE-STREAMING-X`.
- Direct parent: N04 V017 source commit `6121b808b`, SHA-256 `786b0100c88adb232219fd221c4ffeea68ee393dc692405a41cbaf3cb97e646f`.
- Candidate source commit: `f4be331e4`.
- Candidate source path: `本地实验/N04-GM-CACHE-STREAMING-X/V018/submission.asc`.
- Candidate SHA-256: `8f3f547622f39667fe0de8b527db617332577688722c4beab3b621e9833d77bc`.
- Single change: in the wide low-precision path, `residualGm_` hint changes from `CACHE_MODE_PERSISTENT` to `CACHE_MODE_NORMAL`. Gamma, bias, x, and output hints stay as in V017. Arithmetic, Vector instructions, Scalar indexing, tile traversal, DMA descriptors, and parameter residency are unchanged.

## Compile

- Host: `hwnput3` via `cann-server3`; CANN `8.5.0.alpha002`; SoC `Ascend910B3`.
- Parent for the paired Runner: R31B V011 Champion, SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Existing `paired_runner_v016` CMake and Runner entry passed configure and Compile. CMake input paths and remote hashes are in `build-identity.txt` and `remote-inputs.txt`.
- Runner executable SHA-256: `06ed29ab35b9deee8b29dfcb64582347008066c2af692ee1b4fb36fed31b3980`; saved local artifact: `artifacts/paired_runner_v016`.
- `/home/data4t2` reported 100% use and approximately 16 GiB available before and after build. The pre-build d7/disk snapshot is `../pre-build-d7-and-space.txt`. No unrelated data was removed or changed.

## Correctness

- PASS for all four FP16/BF16 tail and wide cases on d7, with R31B V011 Champion and N04 V018. Raw output: `correctness-v011-v018.log`; final line reports `RUNNER_COMPLETE mode=CORRECTNESS correctness=PASS cases=4 device=7`.
- The immediate pre-run snapshot at `pre-correctness-d7.txt` showed HBM use 37%, AICore/AIVector/HBM bandwidth at 0%, and shared Python PID 439848 using 21,260 MiB. After Correctness, `post-correctness-d7.txt` showed AICore/AIVector 11%, HBM bandwidth 8%, and the same process.

## Paired Local

- NOT RUN; `LOCAL_SCORE=NONE`. Do not infer performance from Correctness.
- At 00:15:44, d7 briefly read AICore/AIVector/HBM bandwidth at 0%; the immediately following 00:16:42 launch snapshot showed AIVector at 48%. A further read at 00:17:47 showed AIVector 90% and HBM bandwidth 30%. Shared Python PID 439848 remained at 21,260 MiB. The activity was left untouched and no timed Runner was launched.
- Revisit with a fresh d7 snapshot before a later paired Local. Use the existing Runner for 45 warmups and 21 alternating pairs per case only when shared activity is suitable.

## Evidence

- Configure/build: `configure.log`, `paired-build.log`, `build-identity.txt`.
- Source and Runner inputs: `remote-inputs.txt`, `paired_runner_v016.asc`, `paired_runner.cpp`, `paired_runner_abi.h`, `paired-CMakeLists.txt`.
- Device/result files: `../pre-build-d7-and-space.txt`, `pre-correctness-d7.txt`, `correctness-v011-v018.log`, `post-correctness-d7.txt`, `pre-local-d7-recheck1.txt`, `pre-local-d7-launch.txt`, `pre-local-d7-recheck2.txt`.
- Runner binary: `artifacts/paired_runner_v016`.
