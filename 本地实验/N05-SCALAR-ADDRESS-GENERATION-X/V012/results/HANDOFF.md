# N05 V012 Handoff

V012 source commit is `5ce3904a554eb9fc2c2db90a8b72198b3739597a`, based on V011 source commit `ec078b52b0ab94b8b80bfd22a7793ce470aab6d3`. The kernel change updates only pass-two gamma/bias next-tile GM prefetch offset generation in ProcessWideLowPrecision.

Compile and Correctness passed on d2 with CANN 8.5.0.alpha002. Paired Local completed with 21 pairs per case across four cases. The complete raw output and d2 snapshots are retained. Large variation leaves LOCAL_SCORE=NONE; per-case medians and paired-delta ranges are in RESULTS.md.

Next revision: continue on this Route branch with one different Scalar address-generation change in ProcessWideLowPrecision, using the existing d2 Compile/Correctness/Local entry.
