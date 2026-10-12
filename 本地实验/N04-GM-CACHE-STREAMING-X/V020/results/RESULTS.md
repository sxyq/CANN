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
