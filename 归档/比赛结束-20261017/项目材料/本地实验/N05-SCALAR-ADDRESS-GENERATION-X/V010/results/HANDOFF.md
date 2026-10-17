# N05 V010 Handoff

V010 source commit is `531158beb674dcc6e28cea2e0de13854b70c22eb`, based on V009 source commit `768dd8b88ec5993db984f01e95a9472d98ce5980`. The only kernel change replaces the pass-two retained-y UB row-offset table with a per-tile offset cursor in ProcessWideLowPrecision.

Compile and Correctness passed on d2 with CANN 8.5.0.alpha002. Paired Local completed with 21 pairs in each of four cases. The complete raw rows are preserved in `local.raw.log`; spread leaves LOCAL_SCORE=NONE. The runner's Parent/Candidate labels map to V009/V010 as stated in build-identity.txt.

Next revision: test a distinct Scalar GM input-address generation factor in ProcessWideLowPrecision, using the existing d2 Compile/Correctness/Local path.
