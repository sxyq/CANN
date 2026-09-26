EXTERNAL_IDEA: Phase-based scratch / shared-memory role reuse (compiler-style lifetime reuse applied to AI Core UB)
SOURCE: Public CS practice — register/scratch lifetime reuse; CUDA shared-memory multi-phase reuse (conceptual only; no external source copied)
MECHANISM: Single physical UB pool; at phase boundaries the same byte range changes role (INGEST tiles → freed after fuse → EMIT out staging), while PARAM (gamma/bias FP32) and ReduceSum tmp stay anchored so peak live-set shrinks.
WHY_DIFFERENT_FROM_EXISTING_29: Pool is not partitioned once into permanent roles; identity of bytes across phases is the experimental variable. R030 only reduced dedicated wide-param footprint; R013 adds slots without freeing permanent roles.
EXPECTED_BOTTLENECK: Peak UB live-set forcing small tiles and gamma/bias reloads; reducing peak live-set should raise effective tile and param residency under the same 184 KiB cap.
PROVENANCE_CLASS: PUBLIC_CONCEPT_NO_CODE
