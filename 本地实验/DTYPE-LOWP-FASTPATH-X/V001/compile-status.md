# V001 Compile Status

ROUTE=DTYPE-LOWP-FASTPATH-X
REVISION=V001
SOURCE_COMMIT=fccb0d62220214ab8b60fb21668cfd0bb6d6dfab
SOURCE_SHA256=d9297e337f4aee09994ff6d7f41ad3d55ff172a1382ee042b44f791dea114c1c
DEVICE=3
HOST=hwnput3
BUILD=PASS
LINK=PASS
EXECUTABLE_SHA256=54d687cd50797f3432f24953915e7e5e19c79a9650a9ef549b0451caf72e3e1d
CORRECTNESS=INCOMPLETE
LOCAL=MEASUREMENT_BLOCKED
BLOCKER=CORRECTNESS_RUNNER_UNAVAILABLE

The formal current-toolchain device compile, submission compile, and full link passed
with the CANN wrapper/include chain in `support/`. The earlier bare syntax-only attempt
failed on `cmath` and is retained as historical diagnostic evidence. No Candidate source
was modified.

COMMANDS:
- `bisheng -fsyntax-only -x c++ -std=c++17 --npu-arch=dav-2201 ... submission.asc`
- `bisheng -fsyntax-only -x c++ -std=c++17 --npu-soc=Ascend910B3 ... submission.asc`

The full-link executable SHA is recorded above. The host smoke is not correctness
evidence because its null pointers are rejected before an NPU launch.
