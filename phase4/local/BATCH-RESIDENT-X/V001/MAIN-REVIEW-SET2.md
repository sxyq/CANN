# Main Review — BATCH-RESIDENT-X V001 set2 (dev4)
SHA frozen ad961c58…; CORRECTNESS PASS batch path
LOAD=DEV4_FREE_WINDOW_VLLM_RESIDENT; concurrent ALIGN-TAIL-X observed on d4
PROBE_DELTAS [-41.88, -46.68, -5.97, +30.22]% DIR=3/4 MEDIAN=-23.93% CONF=LOW
decision=NEEDS_ONE_MORE_LOCAL
Not ONLINE (inconsistent + concurrent). Not REJECT (3/4 favor V001).
Shape-dependent: 128x128 +30% loss vs large wins elsewhere.
