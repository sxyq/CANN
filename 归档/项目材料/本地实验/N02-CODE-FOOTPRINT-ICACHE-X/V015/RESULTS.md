# N02 CODE-FOOTPRINT-ICACHE-X V015

| Field | Value |
|---|---|
| ROUTE / REVISION | N02 / V015 |
| PARENT | R31B V011; source SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` |
| PARENT_ARTIFACT_SOURCE | V014 retained Parent device object and `.alink`; object byte-compares equal to V014 Parent |
| CANDIDATE_SOURCE_COMMIT | `e89597723a19c60d9b05e6889e3adc96b33f1987` |
| CANDIDATE_SOURCE_SHA256 | `531751ef88ca63ec4adb4da24b2089b6b429a4b08cb4ee1da88f826479c468a4` |
| RUNNER | Existing R31B V016 paired Runner; SHA-256 `3c7e855cb5127ee3c28dd60aa3e3a5d4cfdd9f7895809d596cdf3424684c1fc6` |
| DEVICE / SOC / TOOLCHAIN | d6 / Ascend910B3 / CANN 8.5.0.alpha002 |
| REMOTE_BUILD | `cann-server3:/tmp/n02-v015-e89597723.ACK0hV`; temporary directory removed after evidence retrieval |

## Change

V015 changes only the pass-2 wide low-precision branch layout in `ProcessWideLowPrecision`. A predicted-cold `tileCount <= 1` branch jumps forward to the existing terminal-tile block. The multi-tile path falls through tile zero, the interior loop, and then the terminal block. Per-tile loads, prefetch target, buffer selection, waits, Vector operations, stores, and event order are unchanged from V014. No inline/noinline, address calculation, tile width, arithmetic, or dispatch change was made.

The current wide dispatch has `rowWidth > 8192` and tile width 4096, so these exercised shapes have at least three tiles. Timing cannot establish that the compiler honored the intended branch placement. Control-flow layout can also change instruction scheduling.

## Compile And Artifact Identity

Candidate configure and `full_link` returned 0. The existing V016 paired Runner configure and build returned 0. The generated dependency file names both `线上结果/R31B/V011/submission.asc` and V015 `submission.asc`.

| Artifact | Size | SHA-256 |
|---|---:|---|
| V015 `candidate-device.cpp.o` | 346,024 B | `1b5866c6b19331718c20608b4e7e71fc21205d13e75be4925efb3cca5f1f2478` |
| V015 `candidate-device.alink` | 191,704 B | `f1e763c1b5781675934a87a9b96960f08084db6c38da756584de031004272d58` |
| V015 `candidate-submission.alink` | 191,712 B | `be7b7475165762008b493d72753f671464f39e644b2939547942d8a3f5d085c4` |
| Reused V011 Parent object | 334,040 B | `3e3266d0d3abffa5d0a48660f97f9a7c3abf9e8abb485d87617fe97c89c0bb57` |
| Reused V011 Parent `.alink` | 183,144 B | `998896a58bf828357e6885fb5eab3e41b79fcf70e898baeb3abf126f90be1b9f` |
| V015 `paired_runner_v016` | 588,664 B | `3c7e855cb5127ee3c28dd60aa3e3a5d4cfdd9f7895809d596cdf3424684c1fc6` |

The V015 Parent object byte-compares equal to the retained V014 Parent. Candidate object, device `.alink`, and submission `.alink` differ byte-for-byte from their V014 counterparts. Candidate object size is 8 B larger than V014; device `.alink` size is unchanged. These sizes describe whole artifacts, not the hot instruction region. No usable device disassembly or ICache counter was collected.

## Correctness

The existing paired Runner completed on d6 with RC=0. Parent V011 and Candidate V015 both passed all four local cases: FP16/BF16, 2 rows, widths 12288 and 32768. Raw output is in `results/correctness/paired-correctness.log`.

## Same-Binary Control

The existing `--qualify-v011` mode collected 62 samples per case (31 in each of two blocks). Robust MAD and block-drift classifications passed all four cases, RC=0:

| Case | Median us | MAD / median | Block drift |
|---|---:|---:|---:|
| FP16, D=12288 | 6.130 | 0.05873 | 0.01305 |
| FP16, D=32768 | 10.680 | 0.03558 | 0.04120 |
| BF16, D=12288 | 6.830 | 0.03221 | 0.04978 |
| BF16, D=32768 | 13.950 | 0.02437 | 0.03154 |

Raw control samples still contain event-time spikes; the largest all-sample value was 210.720 us. No sample was removed. See `results/control/same-binary-v011.log`.

## Paired Local

The existing `--paired` mode completed 21 alternating Parent/Candidate pairs per case, 84 pairs total, RC=0. All samples are retained.

| Case | Parent median us | Candidate median us | Delta median us | Delta p10..p90 us | Delta min..max us |
|---|---:|---:|---:|---:|---:|
| FP16 tail, D=12288 | 6.220 | 7.440 | +0.120 | -54.560..74.480 | -116.860..159.980 |
| FP16 wide, D=32768 | 12.420 | 10.040 | -0.380 | -45.720..101.440 | -133.120..151.080 |
| BF16 tail, D=12288 | 6.640 | 6.520 | -0.140 | -9.240..62.100 | -84.020..123.720 |
| BF16 wide, D=32768 | 14.060 | 13.300 | -1.040 | -31.940..6.760 | -125.780..165.440 |

Paired deltas have large tails; the same-binary series also has large spikes. d6 snapshots show HBM at 59,876-59,878 / 65,536 MB, AICore at 0%, and VLLMWorker_TP PID 3357697 using 56,355 MB with about 9.7% host CPU. The process was left untouched. The measurements do not support stable attribution.

- LOAD_QUALITY: LOW
- LOCAL_SCORE: NONE
- LOCAL_DELTA: NONE
- INTERPRETATION: Concurrent VLLM load and extreme event-time tails prevent a performance conclusion. No ICache improvement is claimed; control-flow and scheduling effects are not separated.

## Storage And Official Results

The `/home/data4t2` filesystem reported 100% capacity with about 6.8 GB available. No writes were made there. Build and run output was kept under `/tmp` on `/`, which had about 84 GB available, then copied to this V015 results directory. The temporary remote directory was removed after retrieval. No Judge submission was made. Official testcase shape mapping remains unknown; no official case is attributed to these four local shapes.

## Evidence Paths

- Compile status, logs, dependency binding, and artifact identity: `results/compile/`
- Candidate, reused Parent, and Runner binaries: `results/artifacts/`
- Correctness output and status: `results/correctness/`
- Same-binary raw samples and status: `results/control/`
- Paired raw samples and status: `results/local/`
- d6, process, and filesystem snapshots: `results/snapshots/`
