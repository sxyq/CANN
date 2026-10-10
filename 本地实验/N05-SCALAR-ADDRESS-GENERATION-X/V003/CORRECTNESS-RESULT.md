# N05 V003 Correctness

CORRECTNESS=NONE
TARGET_DEVICE_ID=3
STANDALONE_V002_PROBE_BUILD=PASS; EXECUTION=NOT_RUN
STANDALONE_V003_PROBE_BUILD=PASS; EXECUTION=NOT_RUN
PAIRED_V002_V003_RUNNER_BUILD=PASS_AFTER_RETRY; EXECUTION=NOT_RUN
R31B_V011_COMPARISON=NOT_RUN
CORRECTNESS_RAW_OUTPUT=NONE
DEVICE_EXECUTION=NO

The pre-run NPU3 snapshot showed no running process and HBM usage of 3429 MiB. No correctness executable was started, so Parent, Candidate, and R31B V011 have no device result. The first paired build error and later successful build are retained in the two runtime build logs; build success does not provide a correctness result.
