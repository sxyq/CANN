# ROW-SCALE-HOIST-X V058 finite retest

- Evidence source commit: `909f8641d76baa3b0643afcaba890f4b5b8733d2`
- Source object: `909f8641d76baa3b0643afcaba890f4b5b8733d2:本地实验/ROW-SCALE-HOIST-X/V058/submission.asc`
- Candidate SHA-256: `6c4a11c570226ec17821063c33c6e5f48f1f9a0ec7c9f5b825c38c766ef7b20d`
- Direct Parent: V026; Parent SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`
- Compile: PASS for the existing `device` and `submission` targets. Fresh correctness/local runner builds used the existing route-local CMake entries with the proven `ASC_DIR` package path.
- Correctness: PASS for the retained route-local case FP16 `[128,128]`; dispatch `ProcessSmallLowPrecisionContiguousBatched_FP16`; Parent and Candidate matched ratio `1.0`; max absolute errors `0.0009765625` and `0.001953125`; Candidate delta executed.
- Local: all 9 invocations exited 0; 45 warmups and 32 device-event samples per invocation; 128 pooled samples per arm.
- Formula: `score = 100 * pooled Parent median device_us / pooled Candidate median device_us`; `delta = 100 * (Candidate / Parent - 1)`; positive delta means Candidate slower.
- Numeric result: Parent pooled median `18.379999 us`, mean `18.2115625234 us`; Candidate pooled median `18.53 us`, mean `21.1201562266 us`; score `99.1904964922`; Candidate delta `+0.8161099465%`.
- Paired block medians: `14.559999/17.0599995`, `19.089999/19.7699995`, `21.79/17.7700005`, `17.92/16.839999`; Candidate faster in `2/4` blocks.
- Quality: `MEASUREMENT_BLOCKED`; Parent pooled CV `0.4243707627`, Candidate pooled CV `0.5721125640`, Parent stability CV `0.5298480499`; Candidate maximum raw sample `101 us`; concurrent processes remained untouched.
- Current Local Best: V026. This descriptive result is not Official-comparable and does not authorize Online.
- Exportable: `YES` as an experimental route-local candidate package; Online: `NOT_RUN`.

Raw and load evidence are retained in this directory, including `device-snapshot-before-local.log`, `device-snapshot-after-local.log`, `device-release.log`, `local-invocation-status.tsv`, and all nine runner logs. The initial ASC configure failure and corrected retry are also preserved in `correctness-build.log` and `correctness-build-retry.log`.
