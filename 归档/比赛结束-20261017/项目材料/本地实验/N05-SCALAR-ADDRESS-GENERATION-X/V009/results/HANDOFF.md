# N05 V009 Handoff

V009 source commit is `768dd8b88ec5993db984f01e95a9472d98ce5980`, based on V008 source commit `8f2f031e3f2884b663793f91a1782c5daabe53da`. The kernel change updates only pass-two output GM offset generation in ProcessWideLowPrecision. The existing V001 paired runner was reused on d2.

Compile and Correctness passed on CANN 8.5.0.alpha002. The initial loader error and both successful same-binary retries are preserved in `correctness.raw.log`. Paired Local completed on d2 with 21 pairs per case; high load variation leaves `LOCAL_SCORE=NONE`.

The V008 Direct-Parent raw measurement logs and identity files were committed with the V009 source. V009 compile, correctness, Local output, and d2 snapshots are saved alongside this handoff.

Next revision: continue on this Route branch with one different executable Scalar address-generation change in ProcessWideLowPrecision. Use the existing CANN 8.5 Compile/Correctness/Local entry on d2 and preserve the established traversal, arithmetic, DMA, event, and Core ownership behavior.
