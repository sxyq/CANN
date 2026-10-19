# N02 CODE-FOOTPRINT-ICACHE-X V014

| Field | Value |
|---|---|
| ROUTE / REVISION | N02 / V014 |
| PARENT | V013, source commit `3b0593944` |
| CANDIDATE_SOURCE_COMMIT | `eba13c32d` |
| SOURCE_SHA256 | `697b2f181763300a05207df7e9bff373e86e5844b272a99a74fe67a6541b44d6` |
| SOURCE | `/Users/sunyiyang/Desktop/Project/cann/worktrees/n02-code-footprint-icache/本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V014/submission.asc` |
| REMOTE_HOST / RUN | `cann-server3` / `/home/data4t2/lelinfeng/cann/server_runs/N02-CODE-FOOTPRINT-ICACHE-X-RESET-20261011/V014/697b2f181763300a05207df7e9bff373e86e5844b272a99a74fe67a6541b44d6` |
| DEVICE / SOC / TOOLCHAIN | d6 (NPU 6) / Ascend910B3 / CANN 8.5.0.alpha002 |
| PAIRED_RUNNER | V016, SHA-256 `fc161b75687af8a5ec65318b59a5331459677358b963005dcf901fda341d8093` |

## Change

V013 used an explicitly duplicated two-tile loop body. V014 gives tile zero its own prologue, then uses a uniform one-tile interior loop and the existing terminal path. The change targets generated control-flow layout and code volume. Per-tile parameter copies, apply operations, and event calls remain in their established order; no row indexing or address-generation change was introduced. This changes loop structure and may affect compiler scheduling as well as code layout, so any timing effect is not an isolated instruction-cache measurement.

## Compile And Identity

Candidate configure and full-link both returned 0. The compiled Candidate object is 346,016 B and Candidate device `.alink` is 191,704 B; `candidate-submission.alink` is 191,712 B. The reused R31B V011 Parent object is 334,040 B and Parent `.alink` is 183,144 B. Candidate object and `.alink` are byte-different from both Parent and V013. Device artifacts and comparison evidence are in `results/artifacts/` and `results/compile/artifact-comparison.txt`.

The Runner dependency file `results/compile/paired-runner-inputs.d` names the R31B V011 Parent source and V014 Candidate source. The ASC wrapper also includes V014. The existing Runner artifact is 588,200 B with SHA-256 `fc161b75687af8a5ec65318b59a5331459677358b963005dcf901fda341d8093`.

Build status records describe separate attempts and must not be conflated:

- Candidate full-link: `candidate-build-status.txt`, RC=0.
- Initial paired Runner build: `paired-build-status.txt`, RC=2; `compile-session.log` also records `paired_build_rc=2`.
- Paired Runner retries 01 through 04 failed during environment setup or ASC compilation. Their logs are retained.
- Retry 05: `paired-build-retry-05-status.txt`, RC=0; its raw compiler output ends with `[100%] Built target paired_runner_v016`. This retry produced the Runner artifact and dependency record used below.

## Correctness

The first launch attempt did not start the Runner because the remote `results/correctness` directory was absent; its launch error is retained in `paired-correctness.log`. Attempt 02 reached the loader but exited RC=1 because the selected CANN `libstdc++.so.6` lacked `GLIBCXX_3.4.29`; it did not enter the Runner. Attempt 03 used the system C++ runtime ahead of the CANN toolchain runtime and completed:

- Correctness: PASS, 4/4 Parent V011 and Candidate V014 cases, RC=0.
- Start/end: 2026-10-12 06:44:53-06:44:59 UTC.
- Runner SHA and Candidate source SHA are printed in the attempt-03 record.
- Raw output and status: `results/correctness/paired-correctness-attempt-03.log` and `paired-correctness-attempt-03-status.txt`.

The earlier loader and missing-directory attempts remain unchanged.

## Device Load

Read-only d6 snapshots were saved before Correctness (06:40:28 UTC), Same-binary control (06:48:09 UTC), and Paired Local (07:04:18 UTC), and after Paired Local (07:16:00 UTC), under `results/snapshots/`. The snapshots report HBM at 91%, AICore at 0%, HBM bandwidth at 0%, and VLLMWorker_TP PID 3357697 using 56,355 MB; its host CPU reading was about 9.5%. `fuser /dev/davinci6` reported no device-node holder and no N02 Runner was active at the snapshots. The VLLM process was not touched. After the experiment it remained present with the same reported memory use.

The user supplied a separate 06:17 UTC reading of approximately 9,181 MB free HBM. Later snapshots are retained as reported by `npu-smi`; free HBM was not inferred from the percentage.

## Same-Binary Control

The existing `--qualify-v011` mode ran 62 device-event samples per case (31 in each of two blocks), RC=0. Its robust MAD and block-drift classifications passed all four cases, but the raw series included large event-time spikes. No sample was removed.

| Case | Median us | MAD/median | Block drift | Max us |
|---|---:|---:|---:|---:|
| FP16 tail D=12288 | 5.860 | 0.03413 | 0.03072 | 83.060 |
| FP16 wide D=32768 | 10.000 | 0.02600 | 0.00600 | 152.380 |
| BF16 tail D=12288 | 6.690 | 0.02990 | 0.01794 | 99.100 |
| BF16 wide D=32768 | 13.830 | 0.01302 | 0.01735 | 269.580 |

Raw samples and all summary fields are in `results/control/same-binary-v011.log`; status is `same-binary-v011-status.txt`.

## Paired Local

The existing `--paired` mode completed 21 alternating Parent/Candidate pairs per case, 84 pairs total, RC=0. All raw samples, device-event and wall times, correctness output, and summaries are in `results/local/paired-local.log`; status is `paired-local-status.txt`.

| Case | Parent median us | Candidate median us | Paired delta median us | Delta p10..p90 us | Delta min..max us | Delta MAD us |
|---|---:|---:|---:|---:|---:|---:|
| FP16 tail D=12288 | 6.280 | 6.080 | -0.180 | -0.720..142.240 | -457.160..166.360 | 0.400 |
| FP16 wide D=32768 | 10.340 | 10.000 | -0.200 | -87.620..8.080 | -142.960..64.880 | 0.880 |
| BF16 tail D=12288 | 6.600 | 6.500 | -0.220 | -0.500..0.360 | -249.960..107.920 | 0.100 |
| BF16 wide D=32768 | 13.920 | 13.200 | -0.820 | -24.220..137.920 | -126.540..239.600 | 0.300 |

Paired samples contain many 70-463 us device-event values against typical latencies around 6-14 us. The same-binary robust classification passes, but its raw values also show spikes up to 269.580 us; wall-clock medians range from 162.645 to 320.119 us with broad distributions. The paired delta medians are small beside the observed tails. No outlier was selected or removed.

- LOAD_QUALITY: LOW
- LOCAL_SCORE: NONE
- LOCAL_DELTA: NONE
- INTERPRETATION: Same-binary control and paired data both contain large, unexplained event-time spikes. The device-event distribution is not credible for attributing the small median differences to V014.

The local Runner case labels and shapes are recorded above. No evidence maps these four local cases to Official Case 2, 4, or 15; no Official score attribution is made.

## Evidence Paths

- Compile logs, status, dependency record, artifact comparison: `results/compile/`
- Candidate, Parent, and Runner binaries: `results/artifacts/`
- Correctness attempts and statuses: `results/correctness/`
- Same-binary control raw samples and status: `results/control/`
- Paired raw samples and status: `results/local/`
- d6 before/after snapshots: `results/snapshots/`
