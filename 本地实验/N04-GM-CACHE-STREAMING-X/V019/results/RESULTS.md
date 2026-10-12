# N04 V019 Results

## Candidate

- Route: `N04-GM-CACHE-STREAMING-X`.
- Direct parent: N04 V018 source commit `f4be331e4`, SHA-256 `8f3f547622f39667fe0de8b527db617332577688722c4beab3b621e9833d77bc`.
- Candidate source commit: `1c80adf17`.
- Candidate source path: `本地实验/N04-GM-CACHE-STREAMING-X/V019/submission.asc`.
- Candidate SHA-256: `13435a1b4b14e5094f99931cfe99647a4cf43603bea8592cf580743870cbcaf5`.
- Single change: in the wide low-precision path, `outputGm_` hint changes from `CACHE_MODE_DISABLE` to `CACHE_MODE_NORMAL`. Gamma, bias, x, and residual hints stay as in V018. Arithmetic, Vector instructions, Scalar indexing, tile traversal, DMA descriptors, and parameter residency are unchanged.

## Compile

- Host: `hwnput3` via `cann-server3`; CANN `8.5.0.alpha002`; SoC `Ascend910B3`.
- Paired Parent: R31B V011 Champion, SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Existing `paired_runner_v016` entry configured and compiled successfully. The CMake input paths and all remote input hashes are recorded in `build-identity.txt` and `remote-inputs.txt`.
- Executable SHA-256: `d3de11ce4967e47175dbc14a47e3596d2bded44d998cb1c22e91e9fe8e4776ce`; local artifact: `artifacts/paired_runner_v016`.
- `/home/data4t2` reported 100% use and about 16 GiB available. No unrelated data was removed or changed.

## Correctness

- PASS for all four FP16/BF16 tail and wide cases on d7, comparing R31B V011 Champion and N04 V019. Raw output: `correctness-v011-v019.log`; it ends with `RUNNER_COMPLETE mode=CORRECTNESS correctness=PASS cases=4 device=7` and `REMOTE_EXIT_CODE=0`.
- Immediate pre-run snapshot: AICore 4%, AIVector 8%, HBM bandwidth 0%; shared Python PID 439848 used 21,260 MiB. Post-run snapshot: AICore/AIVector 0%, HBM bandwidth 8%; same shared process remained.

## Paired Local

- NOT RUN; `LOCAL_SCORE=NONE`.
- Before any timed run, d7 readings varied: `pre-local-d7-current.txt` showed AICore/AIVector 0% with HBM bandwidth 33%; `pre-local-d7-recheck2.txt` later showed AICore 11%, AIVector 44%, HBM bandwidth 0%. PID 439848 remained at 21,260 MiB. This active and changing shared-compute load was left undisturbed; no Local samples were collected.
- Re-evaluate using a fresh d7 snapshot before a later Local run. Reuse the existing Runner with 45 warmups and 21 alternating pairs per case if conditions are suitable.

## Evidence

- Pre-build state: `../pre-build-d7-and-space.txt`.
- Configure/build and identities: `configure.log`, `paired-build.log`, `build-identity.txt`, `remote-inputs.txt`.
- Runner sources and binary: `paired_runner_v016.asc`, `paired_runner.cpp`, `paired_runner_abi.h`, `paired-CMakeLists.txt`, `artifacts/paired_runner_v016`.
- Correctness/device files: `pre-correctness-d7.txt`, `pre-correctness-d7-recheck1.txt`, `pre-correctness-d7-launch.txt`, `correctness-v011-v019.log`, `post-correctness-d7.txt`.
- Local decision snapshots: `pre-local-d7-current.txt`, `pre-local-d7-recheck2.txt`.
