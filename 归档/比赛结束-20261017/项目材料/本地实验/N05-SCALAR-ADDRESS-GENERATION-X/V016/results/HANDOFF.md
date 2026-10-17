# N05 V016 Handoff

V016 source commit is `855e0afc263f25f18fb4ae8acd7fb061056124a3`, based on V015 source commit `37cb7ebfac4f61a6387cecd93cf832874a971c7b`. The pass-one input GM batch base advances by each batch's actual row count.

Compile passed with CANN 8.5.0.alpha002. Correctness passed all four cases on d2. Paired Local completed with 21 pairs per case across four cases. All paired-delta ranges span zero and two cases have broad variation, so LOCAL_SCORE=NONE. Raw data and device snapshots are retained.

Continue the Scalar address-generation/Row-Tile study with one independent address factor per revision, keeping Vector instruction emission unchanged. Use the existing d2 Compile/Correctness/Local entry.
