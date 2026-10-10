# N02 V005 Rejection Record

ROUTE=N02 CODE-FOOTPRINT-ICACHE-X
REVISION=V005
PARENT_REVISION=V004
PARENT_COMMIT=7ccafae7c3b4652f8dcc25ab1e54e4036bc19b70

STATUS=REJECTED_BEFORE_COMPILE
SOURCE_CANDIDATE=本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V005/submission.asc
SOURCE_COMPARISON=The local candidate was temporarily created with a real compile-time output-microkernel refactor, so it was not byte-identical to V004. V004_SOURCE_SHA256=75aef181ceb2822c69fb32107b6cc20ac4dcc88d13a1fd727bb7755785314f2d; V005_SOURCE_SHA256=f339edf4ddff8184bb00e7d612c3a7e8e5fdd886b95e34c37ccf16f701686034.
DISPOSITION=Candidate removed; this commit contains this record only.

EXECUTABLE_CODE_LAYOUT_CHANGE=NONE_PRODUCED
COMPILE=NOT_RUN
CORRECTNESS=NOT_RUN
LOCAL=NONE
PERFORMANCE_CLAIM=NONE
SIMILARITY_CLAIM=NONE
RUNNER=NOT_RUN
DEVICE=NONE

REASON=Per the current stop instruction, no CANN compilation was started and no executable object or alink was generated. Therefore there is no executable code-layout result and no basis for a similarity or performance claim.
NEXT_ACTION=STOP_REPEATED_INLINE_OR_NOOP_VARIANTS; await a separately authorized revision with executable evidence.
