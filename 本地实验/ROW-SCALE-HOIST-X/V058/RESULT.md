# ROW-SCALE-HOIST-X V058 Result

- Parent / Current Local Best: V026 (7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9).
- Change: FP16 row-scale placement across FromFloat in ProcessSmallLowPrecisionContiguousBatched only.
- Candidate SHA-256: 6c4a11c570226ec17821063c33c6e5f48f1f9a0ec7c9f5b825c38c766ef7b20d.
- Compile: PASS for device and submission.
- Correctness: Parent and Candidate PASS on FP16 [128,128]; matched ratio 1.0 for both. Candidate max absolute error 0.001953125; the expected branch and Candidate delta were observed.
- Local: descriptive pooled-median score index 130.8988764045; Candidate median 14.24 us vs Parent 18.64 us, delta -23.6051502146%. Pooled means were 14.7343749844 us Candidate vs 17.8153125313 us Parent, delta -17.2937608671%. 128 raw samples per arm are retained.
- Paired blocks: Candidate delta -18.06196%, +13.71841%, -17.81146%, -40.60073%; 3/4 favor Candidate.
- Quality verdict: MEASUREMENT_BLOCKED. Parent stability CV is 0.5272795511, and its stability median (22.21 us) differs from the pooled Parent median (18.64 us); device 3 also had an unrelated training workload during capture. Numeric data is descriptive only.
- Current Local Best remains V026. No Official comparison, Online, shared-record change, or push.
- Device 3 was explicitly released after the post-capture snapshot; the external process was left untouched.

See compile-result.json, correctness-result.json, local-result.json, the raw logs, snapshots, and device-release.log for full evidence.
