# V001 build final summary

ROUTE=WRITEBACK-TRANSACTION-X
REVISION=V001
EXACT_SOURCE_COMMIT=336c1cc3
EXACT_SOURCE_SHA256=a39fafa5a5d9103ae8fdeefacbaed3156488d7971421a9cde137d3f47c164bf1
DEVICE=0

COMPILE=INCOMPLETE_FOR_FINAL_ASC_CHAIN
LINK=FAIL
CORRECTNESS=NOT_STARTED
LOCAL=NOT_STARTED

The earlier direct device compile evidence is retained in
`logs/exact-v001-compile-target-20261006.log`. The route-local support
harness was then switched to the existing Toolkit ASC CMake chain. The final
retry configured successfully but failed during generated host registration
code with `fatal error: 'vector' file not found`, followed by linker failure.
No executable was produced, so correctness and same-binary timing were not
run. Candidate source was not modified.
