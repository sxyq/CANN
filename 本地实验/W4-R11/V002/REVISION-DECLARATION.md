# W4-R11 V002

Parent: R31B V011

Parent source: `parent.asc`

Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

Single change: keep `blockCount`, row math, tile choices, and active-core count fixed; assign the `extraRows` to the highest-index blocks rather than the lowest-index blocks.

Candidate source: `submission.asc`

Evidence-supported target: 64x8192 FP32. A prior W4-R10 correctness receipt recorded 40 available vector cores for the same Ascend 910B3 device family. With the R31B V011 host rule, this shape yields `blockCount=40`, `baseRows=1`, and `extraRows=24`, so the remainder path is exercised.

Evidence-supported control: 12x8192 FP32. Existing R31B V011 and W4-R10 records include this shape. With 40 vector cores, `blockCount=12` and `extraRows=0`, so the remainder path is inactive.

The harness reads the live vector-core count and reports it. If the target does not satisfy `M > blockCount` and `M % blockCount != 0`, its measurement will not be treated as a remainder-path result.

Measurement order: compile; correctness; exact-shape Parent same-binary qualification; then interleaved Parent/Candidate local samples. Online remains PAUSED. No Official score is claimed.

Previous route record: V001 was recorded as `NOT_CREATED`; V002 is the next unused revision identifier.
