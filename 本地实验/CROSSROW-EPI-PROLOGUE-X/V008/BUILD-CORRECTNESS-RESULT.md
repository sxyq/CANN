# V008 Build and Correctness receipt

AGENT_ID: W5-R01/Zeno  
ROUTE: CROSSROW-EPI-PROLOGUE-X  
REVISION: V008  
BRANCH: research/w5-r01-crossrow-epi-prologue  
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow  
SOURCE_COMMIT: `5bea6f480e8359abb82a69506862f035aded8e21`  
DIRECT_PARENT: R31B/V011 (valid baseline; source SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`)  
CANDIDATE_SOURCE_SHA256: `7c6f620ce231e9ebad71b7c8a37bcc8e2ae45ce56086e6f978f02cd1ca9cce06`

## Build

The first wrapper attempt used `set -u`; sourcing `/usr/local/Ascend/ascend-toolkit/set_env.sh` returned RC=1 on an unset `LD_LIBRARY_PATH`, before CMake was invoked. This is retained at `canonical-paired-20261011/BUILD-ATTEMPT-1.log` and is not a Kernel compile failure.

Retry command:

```text
source /usr/local/Ascend/ascend-toolkit/set_env.sh
export CPLUS_INCLUDE_PATH and CPATH using the existing HCC 7.3.0 include paths
cmake --build /tmp/w5-r01-canonical.Mk9W1S/V003/build --clean-first --target clx_ref_parent_probe clx_ref_candidate_probe --parallel 1
```

CMake process RC=0; both targets reported `Built target`. The outer exec wrapper reported status 1 despite the captured `BUILD_RC=0`; this discrepancy is preserved rather than treated as a compiler failure. The `/tmp/.../V003` path is the pre-existing canonical runner staging directory, not a V003 Candidate.

| Object | Source SHA256 | Artifact SHA256 |
|---|---|---|
| Parent R31B/V011 | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` | `e5d05ac104c99450904a4dda4417d6d5ecbd50d04412bc129622b51a56e2cf64` |
| Candidate V008 | `7c6f620ce231e9ebad71b7c8a37bcc8e2ae45ce56086e6f978f02cd1ca9cce06` | `9c2e0df98f22e9fd4cd6c94bd2743dc08ee98448413014b145bbd96e24c6809c` |

RUNNER_REF_SHA256: `2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7`. Exact captured command/RC and artifact hashes are in `canonical-paired-20261011/BUILD-COMMAND.txt`; full compiler output is in `BUILD.log`.

## Correctness

Runner command form for each side:

```text
source /usr/local/Ascend/ascend-toolkit/set_env.sh
<existing clx_ref_parent_probe|clx_ref_candidate_probe> 7 <rows> <width> <dtype_code> <raw_prefix> 0 1 1 0 1
```

| Case | Shape / dtype | Parent RC | Candidate RC |
|---|---|---:|---:|
| C01 | 1x128 FP16 | 0 | 0 |
| C08 | 8x2048 BF16 | 0 | 0 |
| C11 | 1x8192 FP32 | 0 | 0 |
| C12 | 8x8192 BF16 | 0 | 0 |
| C13 | 128x16384 FP16 | 0 | 0 |
| C14 | 128x16384 BF16 | 0 | 0 |
| C16 | 9x32768 BF16 | 0 | 0 |

Result: `PASS_7_OF_7`, 14/14 Parent/Candidate calls RC=0, all `bad=0`, 14 raw outputs retained. Candidate identity in the preflight, runner selectors, and compiled artifact is the committed `7c6f620c…` source. Per-case logs, RC files, stats, raw outputs, and pre/post device snapshots are in `canonical-correctness-20261011/`.

The aggregate wrapper returned status 1 while the saved case RCs and `CORRECTNESS-SUMMARY.txt` report RC_TOTAL=0/BAD_NONZERO=0. This wrapper discrepancy is recorded; no case-level failure was observed.

Correctness pre-snapshot on NPU7: HBM usage 37% of 65536 MB (about 41 GB free), AICore 0%, AIVector 0%. Local same-binary qualification was not run.
