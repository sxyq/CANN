# W5-R01 CROSSROW-EPI-PROLOGUE-X V004 Build / Correctness

AGENT_ID: W5-R01/Zeno
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V004
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
SOURCE_CHECKPOINT_COMMIT: b2fc0653364788e47082b4ac676eb03cf498bd19
RECORD_CORRECTION_COMMIT: 1d90e841e3d8e64c40fa7cca683112b8b134fab2

## Source and harness identity

PARENT_SOURCE: `V004/Parent.asc`
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
CANDIDATE_SOURCE: `V004/Candidate.asc`
CANDIDATE_SOURCE_SHA256: `642b2ae9482ea14e6cde8ed04b457cffc0f1ae2c2d52b242128e85fe36cfd994`
PARENT_ARTIFACT_SHA256: `e5d05ac104c99450904a4dda4417d6d5ecbd50d04412bc129622b51a56e2cf64`
CANDIDATE_ARTIFACT_SHA256: `5d38a83fbab2475226e2a1b0d073bdcba0d0245c12a0332c69854885ecbf95f5`
RUNNER_REF_SHA256: `2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7`
SUITE_SHA256: `30b6edcd59f19f3e495bb23cb5cf2f41bf5e132a6d96f7b050e32fb6983b0a95`

The temporary canonical `submission.asc` selector was bound to `V004/Candidate.asc` for Build/Correctness and restored to the pre-existing V003 selector afterward. No Runner source or shared tooling was edited.

## Build

The first default-target invocation is preserved in `build-v004-canonical-20261011.log` and returned `RC=2` because stale non-canonical targets `clx_parent_probe`/`clx_candidate_probe` include missing `runner_main.inc`. This is an unrelated fixture-input failure, not a V004 compiler diagnostic.

Successful existing canonical target command:

```text
source /usr/local/Ascend/ascend-toolkit/set_env.sh
cmake --build /tmp/w5-r01-canonical.Mk9W1S/V003/build --clean-first --target clx_ref_parent_probe clx_ref_candidate_probe --parallel 1
```

BUILD: `PASS`, `RC=0`
BUILD_LOG: `V004/build-v004-canonical-attempt2-20261011.log`
PARENT/CANDIDATE_ARTIFACTS: hashes above

## Seven-case Correctness

Command form for each Parent and Candidate case:

```text
<clx_ref_parent_probe|clx_ref_candidate_probe> 7 <rows> <width> <dtype> <raw-prefix> 0 1 1 0 1
```

Cases and result: `C01, C08, C11, C12, C13, C14, C16`, Parent and Candidate each; `14/14` invocations `RC=0`, `bad=0`, one device-event sample. Raw/stats/RC/log files are under `V004/canonical-correctness-20261011/`.

CORRECTNESS: `PASS_7_OF_7`
CORRECTNESS_RC_TOTAL: `0`
DEVICE: `7`
CORRECTNESS_LOAD: pre/post HBM usage 37%; AICore/AIVector and process-query limitations are retained in the device snapshots.
