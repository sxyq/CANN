# N05 V020 Handoff

V020 source commit is `bf514ec41ef0f29cbc91df43025abfa8b64306ec`, based on V019 source commit `1521893105b92c02a47e07cea99fdafc3340a066`. The pass-two next gamma/bias GM offset is now derived from the active tile column.

The paired experiment used Parent V019 SHA256 `57a8b98de7bceae8785e126f62eacc69dc23db91d4b30a56c38631fbef55ab4c` and Candidate V020 SHA256 `17b5586fdc1b38925a643b569d3bfad821fbeaec05d1df20bc3c9c97ba366641`; local and server copies match. CANN 8.5.0.alpha002 compile passed, Correctness passed all four cases on d2, and one paired Local completed with 21 pairs per case.

The paired-delta medians are +1.000, +0.100, +1.280, and -0.200 us. All ranges span zero, and FP16 D=12288 has a large Parent outlier under shared high HBM usage. `LOCAL_SCORE=NONE`; raw samples remain intact.

Continue Scalar address-generation research with one independent factor per revision. Keep Vector emission, tile order, arithmetic, DMA/event order, and Core ownership unchanged. Use the existing server3 paired CMake target and d2 runner.
