# RFH V001 correctness harness

This harness reuses the read-only archived `REDUCE-HIER-X` runner and its
double-precision golden. The archive remains unchanged. `parent.asc` is the
archived R31B V011 source and is checked against the declared parent SHA
`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
`runner_candidate.asc` resolves `submission.asc` from this V001 directory.

The archived runner records legacy wall-clock medians as a side effect. Those
values are ignored and are not Local Score or performance evidence.
