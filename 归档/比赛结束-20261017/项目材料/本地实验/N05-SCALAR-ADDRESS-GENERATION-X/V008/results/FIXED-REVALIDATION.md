# Fixed V008 Revalidation

REVISION: V008 only. No V009 source or candidate was created.
CANDIDATE_SOURCE_COMMIT: 8f2f031e3f2884b663793f91a1782c5daabe53da
CANDIDATE_SOURCE_SHA256: 9a32df08e7054ed71390d4ae3fd85b09433fccd5f032f202d8f8fb36abf12a15
PARENT_SOURCE_COMMIT: 78ff4fa803401fbcbb0e57edb4804a2d79f14816
PARENT_SOURCE_SHA256: 6c26ac328b6650baa110ee94cd1fe5ff41c7ed2493fce0e344c088f848cda12b
EVIDENCE_COMMIT: cd8c840ea37ea2fde851cbdb3d119c456fb000e6 (V008 results-only commit)

COMPILE: Existing V008 CANN 8.5.0.alpha002 build PASS, BUILD_RC=0, completed 2026-10-11T08:16:30+0000. No rebuild was performed. CMakeCache selects the fixed V008 parent and candidate source directories. Remote parent/candidate source SHA match the local files. The compiled object SHA is a9477156532c1d34f70c7ff9ff5005bfd7e8f3d404a67b087423e4c536841352. Paired runner SHA is 112f930ffa3f2e0c3fe57ffbf375237f0167ffd2346267a04ce2b413dbdf42f0 and matches the locally saved runner. Build-input paths, runner source SHA, CMakeCache SHA, and all artifact identities are in fixed-build-identity.txt.

CORRECTNESS: PASS, 4/4 cases for Parent and Candidate (8/8 binary-case results), CORRECTNESS_RC=0. The runner exited with device_reset=NO.
LOCAL_RUNNER: PASS, LOCAL_RUNNER_RC=0, device_reset=NO.
LOCAL_SAMPLES: 4 cases, 21 interleaved pairs per case, 84 paired samples total; 84 Parent and 84 Candidate device-latency samples. warmup_each=45. All raw lines are retained in local-fixed.raw.log.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Pairs |
|---|---:|---:|---:|---:|
| FP16 D=12288 | 10.020 | 10.240 | 0.240 | 21 |
| FP16 D=32768 | 20.020 | 20.000 | -0.340 | 21 |
| BF16 D=12288 | 11.180 | 12.700 | 0.120 | 21 |
| BF16 D=32768 | 27.000 | 27.500 | 1.080 | 21 |

LOCAL_SCORE: NONE. CV exceeded 1 for Parent and Candidate in FP16 D=12288, both binaries in FP16 D=32768 and BF16 D=12288, and Candidate in BF16 D=32768. The corresponding outliers are retained; no value is promoted to a score.

DEVICE: d4 / hwnput3. Snapshot files record the complete npu-smi output and sampled process names/PIDs. Before correctness: d4 HBM 59213/65536 MB used, AICore 62 percent. Before Local and after Local snapshots are preserved separately. Existing VLLM processes were present and left unchanged; no N05 runner remains after the run.

EVIDENCE: correctness-fixed.raw.log, local-fixed.raw.log, fixed-build-identity.txt, pre-correctness-fixed-device.txt, pre-local-fixed-device.txt, and post-local-fixed-device.txt. Original V008 compile/correctness/local logs and snapshots were not overwritten.
