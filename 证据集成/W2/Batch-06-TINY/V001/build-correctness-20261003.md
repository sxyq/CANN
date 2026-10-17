# TINY V001 Build and PROXY Correctness

DATE: 2026-10-03
BUILD: `PASS` (`BUILD_RC=0`)
CORRECTNESS: `PASS` (both declared PROXY cases)
TIMING: `NOT_RUN`

## Source and Executable

- Candidate: `本地实验/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/submission.asc`
- Candidate commit: `ab31036f`
- Candidate SHA256: `4e8abff76486532cab4be6bc6098ea80c048802d285837ad0afb193ff9712024`
- Server copy: `/home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/20261003T101808Z/source/submission.asc` (SHA256 matches)
- Executable: `/home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/20261003T101808Z/build-asc3/tiny_v001_proxy_correctness`
- Executable SHA256: `08fc6c673b56ea677ed23bcceb73fa8ed1f7b8e39475f49bcb768f447a06e49e`
- ELF: AArch64 PIE

## Environment and Commands

- Host: `hwnput3`; device `5`; SoC `Ascend910B3`; target `dav-2201`
- CANN: `8.5.0.alpha002`; Bisheng 15.0.5 / Clang 15
- Pre-run HBM: `60200/65536 MB` used (`5336 MB` free); AICore `43%`; AIVector `29%`; one `VLLMWorker_TP` process (PID `89602`, 56678 MB)
- Build used `support/CMakeLists.txt` as an ASC host/device target and the CANN-provided HCC standard-library include paths via `CPATH`.

```bash
source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh
export CPATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward
cmake -S source/support -B build-asc3 -DCMAKE_BUILD_TYPE=Release
cmake --build build-asc3 --target tiny_v001_proxy_correctness --parallel 1 --verbose
/home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/20261003T101808Z/build-asc3/tiny_v001_proxy_correctness 5
```

The final build returned `0`. The runner returned `0`; no timing was collected.

## PROXY Results

Runtime vector-core count was `A=40`; `M=20`; `B=19`; shape `[20,256]`, FP32. Both cases reused the same device allocations and independent host golden calculation.

| Case | Launch | Branch reached | Return | max_abs | bad |
|---|---:|---|---:|---:|---:|
| `PROXY-TINY-V001-EQUAL-FASTFORM` | `availableCoreNum=40`, `blockCount=20` | `rowCount==blockCount`, `H2_FASTFORM` | `0` | `7.15255737e-07` | `0` |
| `PROXY-TINY-V001-NON-EQUAL-PARENT-FALLBACK` | `availableCoreNum=19`, `blockCount=19` | `rowCount!=blockCount`, `PARENT_FORMULA_FALLBACK` | `0` | `7.15255737e-07` | `0` |

## Server Logs

- Successful compile/link: `/home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/20261003T101808Z/build-cpath-retry.log`
- Runtime snapshot, executable/source identity, both PROXY cases and return code: `/home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/20261003T101808Z/correctness-proxy.log`
- Earlier unsuccessful build attempts remain in their timestamped run directories and were not overwritten.
