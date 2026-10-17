# W4-R15 V002

DATE=2026-10-09
DIRECT_PARENT=W4-R15-V001
PARENT_COMMIT=5249837a179f58596b75a2637d4b2944d15234e1
PARENT_SOURCE_SHA256=90b295ab3c0bceff011ffadd780b1abfc2d412146e2f20bb025158b40889a7f4
SINGLE_CHANGE=In ProcessWideLowPrecision Pass 2, move SyncVToMTE2 from each batch-row iteration to once after the batch-row loop for each tile; retain the tile-level V_MTE2 release event and all MTE3 synchronization.
FOCUS_AXIS=PASS2_VECTOR_TO_MTE2_SYNC_FREQUENCY
HYPOTHESIS=The Pass 2 vector-to-MTE2 phase barrier is repeated for each retained row although gamma/bias buffers are released only after the full batch-row loop. A single barrier at that release point should preserve buffer lifetime while reducing sync instructions when batchRows is greater than one.
FALSIFIER=Compile or Correctness failure, or paired Local results that show no latency reduction.
MEMORY_SAFETY=GM addresses, DataCopy/DataCopyPad selection, UB layout, tensor extents, and stores remain unchanged.
SYNC_SAFETY=Keep SyncVToMTE2 before the existing SetFlag<V_MTE2>(prel); keep per-row SyncVToMTE3 and SyncMTE3ToV around each output store.
OFFICIAL=V001_ONLY; V002_NOT_SUBMITTED
