# W4-R11 V003

Parent: V002

Parent source: `../V002/submission.asc`

Parent SHA256: `21835f22db43ff0d4084b5d82576dd13578377903b4480a4dfbd53825cb8c0f5`

Single change: in the narrow-row `Process` path, keep `blockCount`, `baseRows`, arithmetic, tile choices, and pipelines fixed while moving the contiguous `extraRows` owner interval from the high-index suffix used by V002 to a centered block interval.

Candidate source: `submission.asc`

Target proxy: `64x8192 FP32`, using the existing R11 V002 remainder geometry (`blockCount=40`, `extraRows=24`).

Control proxy: `12x8192 FP32`, where `extraRows=0` and the owner-placement change is inactive.

The proxy shapes are local evidence only. No Official testcase shape is inferred. Measurement order is server3 compile, correctness, Parent same-binary qualification, then interleaved Parent/Candidate Local samples. Online submission is not performed by this Route Agent.
