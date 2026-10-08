# SYNC-BARRIER-ELISION-X V052

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V052
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- CANDIDATE_SOURCE_SHA256: `31cfc8dc07c720750f752d929445b0dc7893fa0882cecadb77adb63752606f56`
- SINGLE_HYPOTHESIS: In the FP16 branch of `ProcessSmallLowPrecisionContiguousBatched`, remove only the `PIPE_V` barrier immediately after `Muls(valueRow, valueRow, invRmsValues[batchRow], width)` and before `FromFloat(outputLocal, valueLocal, totalElems)`.
- SINGLE_CHANGE_BOUNDARY: One vector-pipe barrier deletion from exact R31B-V011; the `Muls`, `FromFloat`, gamma/bias epilogue, output store, adjacent event handoffs, and FP32 branch remain unchanged.
- DUPLICATION_AUDIT: V001-V051 Route-local diff patches contain no deletion at this exact FP16 batched-path site. V014 targets a distinct generic tiled path; V048/V049 target different barriers in the batched function; V046/V047/V050/V051 target distinct event handoffs.
- COMPILE_TARGET: `sync_barrier_elision_v052`
- COMPILE: PASS, target `sync_barrier_elision_v052`, executable SHA256 `12b34db215557e31caf0a363fad24f108b28d8091066180b824e3cb49495e37a`. Two setup-only attempts failed first (missing toolkit environment, then missing C++ include path); final installed-toolkit/CPLUS_INCLUDE_PATH build passed. See `logs/compile-v052-20261008.log`.
- CORRECTNESS: PASS_BITWISE; seven FP16 cases, zero mismatches; runner label `CANDIDATE_V052`, runner executable SHA256 `a09a36ca7defc9bfb96a6aede0c6eb26b99ac4c03b520c51c935d22e029dae2c`. See `logs/correctness-v052-20261008.log`.
- LOCAL: `LOCAL_REJECTED_NOISY`; 62 interleaved device-event pairs, 124 same-binary qualification samples, no filtering. Primary score is `100 * (median(Parent device_us) / median(Candidate device_us) - 1)` in percent, negative means higher Candidate latency. Pooled medians are Parent `16.06 us`, Candidate `17.28 us`, Candidate/Parent ratio `1.075965`, score `-7.060185%`; pooled means are `13.956452 / 15.551290 us`, ratio-of-means score `-10.255347%`. Block 1 medians/means are `16.26/14.415484` vs `18.36/15.020645 us`, ratio score `-11.437908%`; Block 2 `15.86/13.497419` vs `17.18/16.081935 us`, ratio score `-7.683353%`. Qualification Candidate median drift is `71.5827%` and `27.9137%`; no promotion. Full raw samples and load context: `logs/local-v052-device3-20261008T013602Z.log`.
- CURRENT_LOCAL_BEST: `R31B-V011`
- DEVICE_ASSIGNMENT: Main allocated device 3 exclusively to this Route for Correctness and Local through numeric result capture. Fresh pre-use snapshots are `logs/device3-pre-correctness-snapshot-20261008.log` and `logs/device3-pre-correctness-run-snapshot-20261008.log`; post-capture snapshot is in the raw Local log at `2026-10-08T01:41:27Z`. Device 3 was explicitly released at `2026-10-08T01:41:30Z`.
- OFFICIAL / ONLINE: no Official score; `NOT_SUBMITTED`.
