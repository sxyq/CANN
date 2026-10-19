# N05 V011 Handoff

V011 source commit is `ec078b52b0ab94b8b80bfd22a7793ce470aab6d3`, based on V010 source commit `531158beb674dcc6e28cea2e0de13854b70c22eb`. The only kernel change streams pass-one x/residual GM input offsets by each consumed tile's valid length.

Compile and Correctness passed on d2 with CANN 8.5.0.alpha002. Paired Local completed with 21 pairs per case across four cases. Raw rows and device snapshots are retained. Load variation leaves LOCAL_SCORE=NONE.

Next revision: test a distinct Scalar pass-two gamma/bias prefetch address cursor with the existing d2 workflow.
