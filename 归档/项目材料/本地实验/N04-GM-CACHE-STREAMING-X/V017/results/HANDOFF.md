# N04 V017 Handoff

The current Candidate is commit `6121b808b`, source SHA-256 `786b0100c88adb232219fd221c4ffeea68ee393dc692405a41cbaf3cb97e646f`. Relative to V016, only the wide low-precision `xGm_` hint changes from `CACHE_MODE_DISABLE` to `CACHE_MODE_PERSISTENT`.

The existing CANN 8.5 paired entry was configured and rebuilt in `build-paired-reverified-786b`. The first compile attempt lacked the C++ `CPATH` and failed at `<vector>`; its log is retained. The retry with the route's established C++ include paths passed. The rebuilt Runner SHA-256 is `bcedbbf3aab5447b2d277552679f06de86269a2f98baf43bcf608cada5f6d7a4`, and its local artifact is `artifacts/paired_runner_v016-x-persistent-reverified-786b`.

The rebuilt Runner passed all four Champion/V017 Correctness cases on d7. Raw output is `correctness-current-sha-d7.log`; the Candidate and Champion input identities are recorded in `build-identity-current-sha-rebuild.txt`.

Paired Local completed on d7 with 45 warmups and 21 pairs per case. Raw output is `local-paired-current-sha-d7.log`, return code 0. Parent SHA-256 is `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; Candidate SHA-256 is `786b0100c88adb232219fd221c4ffeea68ee393dc692405a41cbaf3cb97e646f`. Paired-delta medians were -16.080 us (FP16 tail, 19/21 negative), -50.200 us (FP16 wide, 19/21 negative), -12.960 us (BF16 tail, 19/21 negative), and -65.280 us (BF16 wide, 12/21 negative). Large signed outliers and load variation, particularly on wide cases, leave the overall result at `LOCAL_SCORE=NONE`; this is one Local run, not an Official result. Post-run d7 recorded AICore 12%, AIVector 4%, HBM bandwidth 59%, with shared PID 439848 still using 21,260 MiB. The process was left untouched.

The earlier `bias-disable` files are preserved and are not evidence for the current Candidate. V016 Local remains unmeasured as documented in its own results.
