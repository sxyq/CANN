# N05 V014 Handoff

V014 source commit is `e550af21b8c3bd08aee25702e3fa98f815dcdb95`, based on V013 source commit `5baa441743d3cfe307223ab0db987b275c60e81f`. The pass-one retained-y address uses a linear Scalar cursor advanced by each tile's valid element count.

Compile passed with CANN 8.5.0.alpha002. Correctness passed all four cases on d2. Paired Local completed with 21 pairs per case across four cases; the mixed paired-delta medians and wide ranges leave LOCAL_SCORE=NONE. Raw data and device snapshots are retained.

Continue the Scalar address-generation/Row-Tile cursor study with one independent address factor per revision, keeping Vector instruction emission unchanged. Use the existing d2 Compile/Correctness/Local entry.
