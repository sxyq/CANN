# N01 V005 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V005
PARENT_REVISION_COMMIT: `e833b161dd2ffede6ba8fda34ae0073d67f85ac9`
SOURCE_COMMIT: `9e74552a6c38e7b829f8edbf64f7fe0dbd85e94c`
SOURCE_SHA256: `a1a2e93cfc0165c43fb7da5de4a9855d64dd2b0952e24a4714da75743f2513ba`
PAIRED_PARENT: R31B V011 Champion, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
DEVICE: d5 / NPU 5 / `hwnput3` / Ascend910B3

## Kernel change

In the FP16 output pass of `ProcessWideLowPrecision`, V005 uses `xFp32` for the widened retained-y value and inverse-RMS scaling after the scalar reduction has finished using that buffer. V004 `valueFp32` scratch selection, arithmetic order, transfers, synchronization, core allocation, and cache policy are unchanged. This tests whether ending the output-pass use of `valueFp32` shortens its live interval.

## Candidate and Runner identity

The local Candidate is `V005/submission.asc`, source commit `9e74552a6c38e7b829f8edbf64f7fe0dbd85e94c`, Source SHA256 `a1a2e93cfc0165c43fb7da5de4a9855d64dd2b0952e24a4714da75743f2513ba`. Its server compile input was:

`/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V005/9e74552a/candidate/R31B-V016-WIDE-TILE-SEED_kernel.asc`

The established CMake target remains `paired_runner_v016`. Its per-run support copy changes only `kDevice` from 7 to 5; the Candidate source is not changed by this device selection. Correctness output labels the Candidate `V001`; paired Local labels it `V016`. Both labels refer to the Candidate path and source identity above. No label-only rerun was performed.

The copied Candidate source SHA256 matched between local and server. The d5 Runner source SHA256 is `4f6701ea7edbbcfd3e410e22c0c3ea39a25bc4996b9bf05be4c422827c56f789`; the transferred Runner executable SHA256 is `07f126f77f4884963d5b6b666f8bb89b2c1974a9c0de54888f764b780783189c`.

## Compile and Correctness

- Configure: PASS, return code `0`.
- Compile: PASS, return code `0`, CANN `8.5.0.alpha002`, target `paired_runner_v016`.
- Correctness: PASS, 4/4 cases on d5. Parent and Candidate passed FP16/BF16 at D=12288 and D=32768; Runner exit code `0`.
- Candidate source and compiled artifacts were transferred to/from the V005 server run. All recorded local/server transfer identities matched.

## Device code

The linked Device ELF is `results/aicore_binary_v005.bin`, SHA256 `e7bcbc4abe5d80c22f3732c5d22f5f772c603d06f979a0200b1a26d341e24479`. The extracted FP16 Candidate function is `results/fp16-candidate-v005.bin`, SHA256 `4e2c81495acd77197d2b803d44283b2e9d10538f65b5e777d80fc28df01d5407`.

Function sizes from the V005 build are FP32 Parent/Candidate `12840/12840`, FP16 `13792/13812`, and BF16 `14428/14428` bytes. Within this build, FP32 functions are byte-identical; FP16 differs and Candidate is 20 bytes larger than Parent; BF16 differs at 3 byte positions. V005 FP16 Candidate is 16 bytes larger than V004 Candidate (`13796 -> 13812`). Parent function bytes differ between the V004 and V005 builds for all three dtypes, so cross-build byte differences do not isolate this source edit. The target `llvm-objdump` exits `0` but emits `<not available>` for instruction words. No register-count or register-improvement claim is made.

## Local

The existing paired Runner completed 21 interleaved pairs for each of four cases, 84 pairs total. All Parent and Candidate outputs passed correctness during the run. Raw samples are retained in `results/local-v005.raw.log`.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta range (us) | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 6.440 | 6.200 | -0.100 | -128.140 to 174.000 | 21 |
| FP16 D=32768 | 10.020 | 10.120 | -0.040 | -0.540 to 156.340 | 21 |
| BF16 D=12288 | 6.700 | 6.760 | +0.040 | -141.480 to 133.180 | 21 |
| BF16 D=32768 | 13.860 | 13.880 | +0.040 | -0.780 to 161.980 | 21 |

`LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`. The large opposing event samples remain in the raw file; no sample was removed and no performance gain is claimed.

## Device state and evidence

The snapshots record NPU 5 HBM `65536 MB` total and about `5659 MB` free. At pre-Correctness (12:51:08 UTC), AICore was `49%`; pre-Compile, pre-Local, and post-Local snapshots showed `0%` AICore. The existing `VLLMWorker_TP` process used `56354 MB` on NPU 5 and was left unchanged. Post-Local HBM usage was `59876 MB`; the paired Runner returned `0`.

`results/` retains build and command logs, raw Correctness and Local output, all device snapshots, the V005 linked Device ELF/text and six function extracts, symbol/size output, target disassembly output, the d5 Runner source and executable, and transfer identity evidence. Extraction attempts and their raw errors are retained in `attempts-v005.log` and the named attempt logs.

Remote run: `~/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V005/9e74552a`

NEXT_ACTION: continue on d5 with V006's separate FP16 y-retention/lifetime hypothesis. Cross-route similarity was not measured by this Route.
