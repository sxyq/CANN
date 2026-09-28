# MAIN APPROVAL — COEFF-LOCALITY-X V002

Date: 2026-09-27
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

## APPROVED SINGLE HYPOTHESIS

H2 — Cross-batch gamma/bias stripe residency, tileElems PINNED at 4096.

V001 failed because 2-deep staging stole UB and dropped tileElems 4096→2560.
V002 must keep tileElems=4096 (or whatever ChooseWideFullYRows would pick
with the original 1-tile io) and use only leftover UB for a K-tile gamma/bias
stripe that survives the batch loop. Skip GM load when tile is in the stripe.

One factor: stripe residency width K. No tileElems change. No multi-row DMA.
No mode/rows-per-block. No dtype split. No reduction/scheduling change.

If leftover UB cannot fit any useful stripe at tileElems=4096, write
NO_UB_BUDGET and stop — do not shrink tileElems again.
