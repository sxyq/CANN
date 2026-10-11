# N05 V015 Handoff

V015 source commit is `37cb7ebfac4f61a6387cecd93cf832874a971c7b`, based on V014 source commit `e550af21b8c3bd08aee25702e3fa98f815dcdb95`. The pass-two output GM tile start uses a Scalar cursor advanced by the tile's valid element count.

Compile passed with CANN 8.5.0.alpha002. Correctness passed all four cases on d2. Paired Local completed with 21 pairs per case across four cases; every paired-delta range spans zero and the wide FP16 case is highly variable, so LOCAL_SCORE=NONE. Raw data and device snapshots are retained.

Continue the Scalar address-generation/Row-Tile study with one independent address factor per revision, keeping Vector instruction emission unchanged. Use the existing d2 Compile/Correctness/Local entry.
