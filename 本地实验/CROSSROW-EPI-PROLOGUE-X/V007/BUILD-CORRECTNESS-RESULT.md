# W5-R01 CROSSROW-EPI-PROLOGUE-X V007 corrected source Build / Correctness

AGENT_ID: W5-R01/Zeno
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V007
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
SOURCE_DECLARATION_COMMIT: 9c6d19fae1373a814a1eb044d7780cac7c2c489a
SOURCE_CORRECTION_COMMIT: 901ecf4c5f30f4761dc6d0ad921e9a7a8071a11d
PARENT_SOURCE_SHA256: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256: c09344af4e9ffb78ca50bf8f23f75a6dca1e24640b803ef1d415777cb46381b5
PARENT_ARTIFACT_SHA256: e5d05ac104c99450904a4dda4417d6d5ecbd50d04412bc129622b51a56e2cf64
CANDIDATE_ARTIFACT_SHA256: 7f7a521b869b9a1e3eaefb23ef9c3daff9853c12aa5002b047d9f568cf9ffcf0
RUNNER_REF_SHA256: 2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7

## Corrected source identity

Candidate SHA `c09344af…` commits the actual prefetch move. Its first gamma/bias `Load` is after the pass-1 per-buffer `V_MTE2` release waits and before the scalar RMS loop. The old build/correctness evidence corresponds to SHA `320436ef…`, where the Load remained after that loop; it is retained separately and is not reused for this result.

## Build

Command: `source /usr/local/Ascend/ascend-toolkit/set_env.sh; export toolkit HCC CPLUS_INCLUDE_PATH/CPATH; cmake --build /tmp/w5-r01-canonical.Mk9W1S/V003/build --clean-first --target clx_ref_parent_probe clx_ref_candidate_probe --parallel 1`

Result: `PASS`, `RC=0`. Log: `build-v007-c09344-canonical-20261011.log`. The temporary `/tmp/.../V003` path is only the existing runner staging directory; source and artifact identities are V007 as listed above. Selector was restored after Build.

## Correctness

Case commands use `<probe> 7 <rows> <width> <dtype> <raw-prefix> 0 1 1 0 1`; each runner invocation sources `/usr/local/Ascend/ascend-toolkit/set_env.sh`.

Suite C01/C08/C11/C12/C13/C14/C16, Parent and Candidate: `PASS_7_OF_7`, 14/14 RC=0, `bad=0`. Corrected-SHA evidence is exclusively under `canonical-correctness-c09344-20261011/`. The earlier 14/14 RC files under `canonical-correctness-20261011/` are legacy SHA `320436ef…` evidence and explicitly excluded.
