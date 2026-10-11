# N05 V017 Handoff

V017 source commit is `97222450df4d33b6920f46f42128306f82907f65`, based on V016 source commit `855e0afc263f25f18fb4ae8acd7fb061056124a3`. The pass-two output GM batch base advances by each batch's actual row count.

Compile passed with CANN 8.5.0.alpha002. Correctness passed all four cases on d2. Paired Local completed with 21 pairs per case across four cases. All paired-delta ranges span zero and two have large excursions, so LOCAL_SCORE=NONE. Raw data and device snapshots are retained.

Continue the Scalar address-generation/Row-Tile study with one independent address factor per revision, keeping Vector instruction emission unchanged. Use the existing d2 Compile/Correctness/Local entry.
