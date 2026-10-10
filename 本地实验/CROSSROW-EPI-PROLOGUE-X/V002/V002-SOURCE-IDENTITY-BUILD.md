# W5-R01 CROSSROW-EPI-PROLOGUE-X V002 source identity rebuild

REVISION: V002
HOST: hwnput3
DATE: 2026-10-10
SCOPE: Route-owned V002 source/build/correctness only; no Champion, other Route, or shared-tooling modification

## Source identity

PARENT_SOURCE_PATH: `V002/Parent.asc`
PARENT_SOURCE_SHA256: `3698c229dfb6a4b336e7ee0506eb1ceaf898c1c37c937c0af7d3eef71fbc685d`
CANDIDATE_SOURCE_PATH: `V002/Candidate.asc`
CANDIDATE_SOURCE_SHA256: `426489bfdc63441f2fd179fdff176875b24d815e6e0420343aeed0b0dab7933f`
DECLARATION_SHA256: `6ade2a0017989eed3c03ff2ef55b9a04b6ff6cd2ab37b5f7e39f251f824abd0f`
OFAT_DIFF_SHA256: `e8e656afd7de4f3c565c5ffe7e84758e44ffb4537ffea1fcbf211e4387c9c9ad`

The existing adapters were temporarily retargeted to `../../V002/Parent.asc` and `../../V002/Candidate.asc`. The existing runner metadata was temporarily labeled Parent V001 / Candidate V002. No kernel, ABI, timing, shape, or runner control-flow change was introduced. The V001 support adapter and runner files were restored after the V002 build/correctness pass.

## Build contract

BUILD_ENTRY: existing `V001/support/CMakeLists.txt` and `V001/support/build`
BUILD_TARGETS: `crossrow_v001_parent`, `crossrow_v001_candidate`, `crossrow_v001_paired_runner`
BUILD_MODE: `cmake --build 本地实验/CROSSROW-EPI-PROLOGUE-X/V001/support/build --clean-first --parallel 1`
TOOLCHAIN_ENV: `source /usr/local/Ascend/ascend-toolkit/set_env.sh`
ASCEND_HOME_PATH: `/usr/local/Ascend/ascend-toolkit/latest`
HCC_INCLUDE_ROOT: `${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0`
INCLUDE_ENV: `CPLUS_INCLUDE_PATH` and `CPATH` set to the HCC root, its `aarch64-target-linux-gnu` child, and `backward`
CMAKE_SHA256: `0afde66bbb3f86fa2521264f7673a403bf99dda6f3366022048cb34c14566de3`
CMAKE_CACHE_SHA256: `cf79b98e8eca0201fa1ba75c9740a49b02b25ac0e718152d6b20b5ef0ba9b61c`

V002_BUILD_RC: 0
V002_BUILD_LOG: `rebuild-v002-source-identity-env3.log`
V002_BUILD_LOG_SHA256: `25d71e85c096e39b806f3b12c8af307ebe572966dfae975a335faebdb4c728e7`

NEW_REBUILT_PARENT_SO_SHA256: `3ad998adbb7a126f15b7da0a8f8e0a816ca719d92a4dc42aba97d7832ffa4db4`
NEW_REBUILT_CANDIDATE_SO_SHA256: `0ef46469d226225c47ca4c33286b505b48444175095572bc3abf0e8d49c30126`
NEW_REBUILT_RUNNER_SHA256: `19a6ad5163d9446c849c1516cd8fe15c6ba53b0fc5b2b99730ca57bff95fa2db`
V002_PARENT_ADAPTER_SHA256: `c43679ad324c1f847c4d9a119906c8c1f4ae73bf3c785d328c2300ef6c275a86`
V002_CANDIDATE_ADAPTER_SHA256: `dc64ba3d0cb280d964932fc3408dc3c442e171302562485a4c84f378e7523147`
V002_RUNNER_SOURCE_SHA256: `faf33e8b3f799ef52a4f9d5699a456217c1adc55f438a426e966459d77912016`
V002_RUNNER_ABI_SHA256: `0897a04ba82bda39d53e77d548966eb54a6b7ec16c17387dccf26d86cbf577d2`

These are hashes of the newly rebuilt outputs and temporary V002-bound support sources. They are not copied from the prior V002 manifest.

## Build failures preserved

- `rebuild-v002-source-identity.log`: rc=2, missing C++ `vector` include without environment contract.
- `rebuild-v002-source-identity-env.log`: rc=2, requested fixed-version `set_env.sh` path absent; `ASCEND_HOME_PATH` empty and kernel include unavailable.
- `rebuild-v002-source-identity-env2.log`: rc=2, toolkit environment loaded but HCC include variables absent; missing C++ `vector`.
- `rebuild-v002-source-identity-env3.log`: rc=0 with the recorded include environment.

## Correctness

CORRECTNESS_DEVICE: 3
TARGET: `rows=16,width=6144,blocks=8,dtype=fp16`
CORRECTNESS_COMMAND_TARGET: `source /usr/local/Ascend/ascend-toolkit/set_env.sh; runner --mode correctness --device 3 --rows 16 --width 6144 --blocks 8 --dtype fp16`
TARGET_RESULT: PASS; `bit_differences=0`, `max_abs=0`, `tolerance_failures=0`, `nonfinite=0`
TARGET_LOG: `correctness-v002-rebuild-r16-d6144.log`
TARGET_LOG_SHA256: `24d783886b990392ddc894c2c98899ecc36ca7ba3676f60124938b5c4d8ffb59f`
M1_CONTROL: `rows=1,width=6144,blocks=1,dtype=fp16`
CORRECTNESS_COMMAND_M1: `source /usr/local/Ascend/ascend-toolkit/set_env.sh; runner --mode correctness --device 3 --rows 1 --width 6144 --blocks 1 --dtype fp16`
M1_RESULT: PASS; `bit_differences=0`, `max_abs=0`, `tolerance_failures=0`, `nonfinite=0`
M1_LOG: `correctness-v002-rebuild-m1-d6144.log`
M1_LOG_SHA256: `4c4da3ce1be24360f5fa9c5c71485d1a23f1cabe4133af1ee28d565dad99395f`

## Restore and Local gate

RESTORE_V001_SUPPORT: PASS; `restore-v001-after-v002.log`
RESTORED_PARENT_SO_SHA256: `61eb3391924fabbc7b72231652914df2688ca28d376ed143b5d351042b93e944`
RESTORED_CANDIDATE_SO_SHA256: `278dbd1cd0866f30ce25b2ea9a3c5fd5214b641bc8ae78e3de52b6d35c3ff34b`
RESTORED_RUNNER_SHA256: `5210262b45c50878c24def38e108386b45bd017086a61a1293ab290d171a4d5a`
RESTORED_SUPPORT_DIFF: empty

LEASE_STATUS: NO_AUTHORIZED_R1_V002_LEASE; no current owner/request was visible; NPU2/3 had external process activity in the final snapshot
LOCAL: BLOCKED_NO_AUTHORIZED_LEASE
LOCAL_RESULT: NOT_RUN
LOCAL_VERDICT: LOCAL_NO_PROMOTION / MEASUREMENT_BLOCKED_FOR_PERFORMANCE_AUTHENTICITY
RAW_PRESERVATION: prior contaminated V002 raw unchanged
NEXT_OFAT: none; obtain a real authorized lease, rebuild/verify the exact V002 bundle, then perform at most one clean paired Local. Do not create V003.
