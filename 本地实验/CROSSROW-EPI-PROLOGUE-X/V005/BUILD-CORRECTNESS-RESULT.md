# W5-R01 CROSSROW-EPI-PROLOGUE-X V005 Build / Correctness

AGENT_ID: W5-R01/Zeno
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V005
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
SOURCE_COMMIT: eb1334db2ee928981515b52f9e33b29c2bd1fcd0
FIX_COMMIT: 01c48fa49ef7c8cbe650353bdc8753d95bf38ec5

PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
CANDIDATE_SOURCE_SHA256: `a33c185efc618bd8a588e3ea41031bdb9074e18e89b16966421d7b369d35c2a6`
PARENT_ARTIFACT_SHA256: `e5d05ac104c99450904a4dda4417d6d5ecbd50d04412bc129622b51a56e2cf64`
CANDIDATE_ARTIFACT_SHA256: `8770dba6b057f8ba2d33dc402b1397c927c265e7fd1f74dd3ed9764248f53de0`
RUNNER_REF_SHA256: `2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7`

## Build

COMMAND: `source /usr/local/Ascend/ascend-toolkit/set_env.sh; cmake --build /tmp/w5-r01-canonical.Mk9W1S/V003/build --clean-first --target clx_ref_parent_probe clx_ref_candidate_probe --parallel 1`
BUILD: `PASS`, `RC=0`
BUILD_LOG: `build-v005-canonical-fix-20261011.log`

The first V005 build also passed before the block-length fix and produced `7f69c29f...`; the final rebuilt Candidate artifact is recorded above. The original C13 runtime failure remains under `canonical-correctness-20261011/`.

## Correctness

Final command form for each Parent/Candidate case: `<probe> 7 <rows> <width> <dtype> <raw-prefix> 0 1 1 0 1`.

Final suite: C01/C08/C11/C12/C13/C14/C16, Parent and Candidate each. Result: `PASS_7_OF_7`, `14/14` invocations `RC=0`, `bad=0`; evidence: `canonical-correctness-fix-20261011/`.

The earlier Candidate C13 run with the initial DataCopy byte-length contract returned `RC=1`, ACL `507035` at runner stream synchronization; it is preserved under `canonical-correctness-20261011/` and was not counted as a passing run.
