# STORE-EPILOGUE-W2-X V001 Correctness Diagnostic Run 003

DIAGNOSTIC_STATUS=UNRESOLVED_BOTH_VARIANTS_NONDETERMINISTIC
CORRECTNESS_PASS=NO
TIMING_RUN=NO

## Fixed Identity

- Route branch checkout on server3, resolved from `git worktree list`: `/home/data4t2/lelinfeng/cann-w2-m1-store`, HEAD `5b632c4da540cb3d711d72c1c2b9843bae759eba`.
- Candidate source commit: `a1ebfc125ce215fc81ffbe04f582d5f8682e3294`.
- Candidate `submission.asc` SHA256: `48b9428dc2fc97c7c9d95f03ad8cec8e758c88e1197aa2328b1b1edebc018f88`.
- Direct Parent `parent.asc` SHA256: `59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839`.
- Parent executable SHA256: `a55a4859dd22970216405826c7cb7f37fef4678c491979631505b0aac3f573a7`.
- Candidate executable SHA256: `469522ba548c001e4ba223eb069d170fa0e73a8285f9f5aaaab696a5edd45328`.
- `ldd` with `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64`: all Parent and Candidate dependencies resolved.
- Run-002 and run-003 output directories were found from existing V001 records. Run-003 was confirmed absent before creation; run-001 and run-002 were not written or removed.

## Inputs and Method

Only the fixed device 5 cases `(rows=1, width=16384, dtype=0)` and `(rows=1, width=32768, dtype=0)` were run. Each Parent and Candidate executable ran twice in run-003. Inputs and host expected values follow `correctness_main.inc`: FP32 input generation, FP32 gamma/bias generation, double-precision row sum and expected expression. The comparison uses `atol=1e-4`, `rtol=1e-4`; gross mismatch uses `0.05 * (abs(expected) + 1e-3)`. The recalculation reproduced run-002's four `max_abs`, `bad`, and `gross_bad` values from the retained binaries.

Command form, with `D` set to `16384` or `32768` and `P` set to the matching executable and unique output prefix:

```sh
LD_LIBRARY_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64 \
  P 5 1 D 0 PARENT_OR_CANDIDATE_OUTPUT.bin PARENT_OR_CANDIDATE_OUTPUT.tsv
```

The four run-003 invocations per shape were Parent rep1, Candidate rep1, Parent rep2, Candidate rep2. Every executable RC was `3` because the host expected comparison failed. No timing or profiling command was run.

## Per-Element Distributions

All 12 retained outputs have the expected byte length and are finite: no NaN, infinity, or zero values. `bad/gross` count elements beyond the harness tolerances. `bad/4096` reports those element counts in consecutive 4096-element segments.

| Shape | Runner/run | Output SHA256 | Actual min..max | Bad/gross | Abs error P50/P95/P99.9/max | Bad/4096 |
|---|---|---|---:|---:|---|---|
| 1x16384 | Parent run-002 | `882e8207d91a0a93da623efea6ebc7ca803cde732a55ce31ec3c0b95755367b9` | -3.078611..2.944381 | 14328/12633 | 0.1130102/0.4547418/0.8586884/1.099927 | 4095,4092,4094,2047 |
| 1x16384 | Parent run-003 rep1 | `ed5d1618522216c4dd9ef84329c8e53199bf3cb751a332122300203aeb425291` | -3.094042..2.944381 | 14576/11215 | 0.07000023/0.4522838/0.7633877/1.158651 | 4088,3963,4094,2431 |
| 1x16384 | Parent run-003 rep2 | `073f6ae3318188f882c1e6e9c12f0876db796e35b468fd4629fe40165b045524` | -3.078611..2.920753 | 12280/10477 | 0.07/0.4141407/0.7660601/1.272577 | 4091,3071,4095,1023 |
| 1x16384 | Candidate run-002 | `264e87d9aa3104d8636b9a02341677558258576cda7ac515713f9721045e4a31` | -3.038611..2.944381 | 9081/7532 | 0.03022797/0.3229029/0.7117362/1.259598 | 1024,3072,4094,891 |
| 1x16384 | Candidate run-003 rep1 | `69fd9e34e0a48e4105cc415f707ee9367add66693b84abe38d1477af09fd1e6f` | -2.990334..2.951478 | 11896/10528 | 0.1054393/0.433725/0.8255153/1.189598 | 2686,4096,4095,1019 |
| 1x16384 | Candidate run-003 rep2 | `38f813f2b2fa78299fca54ffe37d54df4a8f21862fb52f78d51b6434b00e1b62` | -3.005764..2.893608 | 9853/9335 | 0.08349501/0.4141409/0.8716671/1.189598 | 1534,4096,4095,128 |
| 1x32768 | Parent run-002 | `7a076a2c2f909b0bb6fc21919902981c040bdb5a519a8dfa5abcf8c32af0285e` | -3.070888..2.937905 | 24441/23178 | 0.1042331/0.4495067/0.7808475/1.2031 | 3072,2047,2944,4093,4093,4096,4096,0 |
| 1x32768 | Parent run-003 rep1 | `7231f1d5a3a49a4fec19496d3ae5c5f55a1822980b3c226d1fad3b8356c1979e` | -3.016313..2.960704 | 27254/24379 | 0.08619196/0.4344963/0.8324373/1.183385 | 2815,4096,4096,3071,4091,4096,4093,896 |
| 1x32768 | Parent run-003 rep2 | `abdb392974248978c02ecc7407a22d10c850dd67ec07bd6bc7e5d7ba5be7425f` | -3.089188..2.952865 | 29799/24839 | 0.11/0.4577788/0.8190188/1.159205 | 4095,3195,4090,4095,4091,4091,4094,2048 |
| 1x32768 | Candidate run-002 | `b0dd95913e645ceabc88b980ebf098e76b3dc6c9df3653db89ef1da3b2465987` | -3.070888..3.030704 | 28017/23884 | 0.09775284/0.4274314/0.9103471/1.2031 | 4096,4096,3067,4094,4090,4095,3455,1024 |
| 1x32768 | Candidate run-003 rep1 | `8a16b9f1aa434a2db9228c168b52ecc8301d484cb7e27669a578ac18a20f6a3f` | -3.070888..2.960704 | 26608/23339 | 0.1015401/0.4370225/0.8207682/1.2731 | 4096,4094,2047,3070,4090,4096,3072,2043 |
| 1x32768 | Candidate run-003 rep2 | `55ca444396b3b9faf61cddf1d8f67be92c37b83b8d8992bdf13c8aba8910494` | -3.070888..2.991425 | 28527/25022 | 0.09049247/0.4488673/0.841663/1.130874 | 4091,4095,4096,4090,4095,3071,3966,1023 |

## Repeatability and Pair Comparison

| Shape | Runner | run-003 reps unequal | run-002 vs rep1 byte-identical |
|---|---|---:|---|
| 1x16384 | Parent | 11776/16384 | no |
| 1x16384 | Candidate | 6525/16384 | no |
| 1x32768 | Parent | 28928/32768 | no |
| 1x32768 | Candidate | 29312/32768 | no |

Each run-003 rep1 Parent/Candidate pair also differs substantially:

- `1x16384`: 12032/16384 elements differ; 11629 indices fail tolerance in both, Parent-only bad `2947`, Candidate-only bad `267`.
- `1x32768`: 28064/32768 elements differ; 22253 indices fail tolerance in both, Parent-only bad `5001`, Candidate-only bad `4355`.

The retained run-002 pair likewise differs: `13567/16384` and `23680/32768` elements. Counts move between repeated runs for both variants, so these Parent/Candidate deltas cannot establish which output is correct.

## Source Review and Conclusion

The only Candidate/Parent source difference is the full-row Store block relocation in `submission.asc` around line 2215. Both shapes use `kWideFullYTileElems=4096`; their tile counts are 4 and 8, and both satisfy `rowWidth % 8 == 0`, so both enter `mergeRowRuns`. The Candidate issues one full-row `DataCopyPad` after the final tile arithmetic; the Parent issues per-tile stores. The copied lengths are 65536 and 131072 bytes. The project's Ascend C API reference gives `DataCopyExtParams.blockLen` a maximum of 2097151 bytes, and both UB row starts are 32-byte aligned; the available evidence does not show a size or alignment violation.

Both runners fail the host expected comparison and both vary heavily across identical repeated inputs. This establishes nondeterminism shared by the Parent and Candidate runs, while leaving its source unresolved. The Candidate changes many elements relative to Parent, but Parent itself is unstable and there is no independent golden for these two cases; the present evidence does not isolate a Candidate-only defect or justify correctness PASS. No Candidate source, executable, other Route, shared record, or timing mechanism was changed.

## Run Artifacts

- Run-002 binaries and per-case records: `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-002/`.
- Run-003 binaries, per-case TSVs, and per-case logs: `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-003/`.
- Run-003 driver log with exact commands, RCs, executable identities, and before/after server state: `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-003.log`.
- Device 5 before run-003: free HBM `4092 MB`; available disk `657838372 KiB`; active PIDs `89602` (`VLLMWorker_TP`) and `2771372` (`python`). After run: HBM `61445/65536 MB`; available disk `657692312 KiB`. No process was stopped or migrated.
