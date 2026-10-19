# N05 V021 Handoff

V021 source commit is `bca94254e8cbfe639c593366a4637addfcd5cb3e`, based on V020 source commit `bf514ec41ef0f29cbc91df43025abfa8b64306ec`. The output GM tile start now comes directly from the batch base and the current tile column.

The paired experiment used Parent V020 SHA256 `17b5586fdc1b38925a643b569d3bfad821fbeaec05d1df20bc3c9c97ba366641` and Candidate V021 SHA256 `035c5e689e0d0a91865b726eb3be7dda747064f471f577b3605dfea95fdd95b4`; local and server copies match. CANN 8.5.0.alpha002 compile passed, Correctness passed all four cases on d2, and one paired Local completed with 21 pairs per case.

The paired-delta medians are +0.780, +1.460, +2.820, and +0.060 us. All ranges span zero under shared d2 load; `LOCAL_SCORE=NONE`. Raw samples and before/after device state remain intact.

Continue Scalar address-generation research with one independent factor per revision. Keep Vector emission, tile order, arithmetic, DMA/event order, and Core ownership unchanged. Use the existing server3 paired CMake target and d2 runner.
