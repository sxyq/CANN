# W4-R15 V001

DATE=2026-10-09
DIRECT_PARENT=R31B-V011
PARENT_COMMIT=43a1049a1e08e518c88e354a754fdebb85a96f99
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
SINGLE_CHANGE=Use ordinary DataCopy for GM-to-UB segments only when byte length, GM source address, and UB destination address are all 32-byte aligned and byte length fits DataCopyParams; keep the existing DataCopyPad path otherwise.
FOCUS_AXIS=ALIGNED_GM_TO_UB_COPY_PRIMITIVE
HYPOTHESIS=On DAV_2201, ordinary DataCopy lowers to copy_gm_to_ubuf while DataCopyPad lowers to copy_gm_to_ubuf_align_b16/b32. The aligned ordinary form may reduce copy issue overhead without changing copied addresses, byte count, UB storage, or consumers.
FALSIFIER=Compile or Correctness failure, or paired Local samples showing no useful latency reduction after including the runtime eligibility checks.
ADDRESS_SAFETY=The ordinary path is selected only for the exact existing source range and an aligned destination; all other calls retain DataCopyPad. No adjacent-row span is added.
LIFETIME_SAFETY=The destination tensor and existing MTE2-to-Vector synchronization remain unchanged.
SHAPE_DTYPE_POLICY=Local proxy cases are explicitly named in runner output; no Case ID is mapped to an inferred shape or dtype.
OFFICIAL=NOT_SUBMITTED
