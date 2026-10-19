# N05 V013 Handoff

V013 source commit is `5baa441743d3cfe307223ab0db987b275c60e81f`, based on V012 source commit `5ce3904a554eb9fc2c2db90a8b72198b3739597a`. The pass-one reduction scratch tile address now uses an advancing Scalar cursor.

Compile passed after restoring the C++ standard include path used by successful N05 builds. Earlier compile failures are retained. Correctness passed all four cases on d2 after setting the existing CANN runtime library paths; the initial loader error is also retained. Paired Local completed with 21 pairs per case across four cases. High spread and mixed paired-delta signs leave LOCAL_SCORE=NONE.

Next revision: test one different Scalar Row-Tile address factor in ProcessWideLowPrecision, with vector instruction emission, arithmetic, DMA/event order, tile order, and Core ownership unchanged. Use the existing d2 CANN 8.5 Compile/Correctness/Local entry.
