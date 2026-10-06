# V001 Correctness Status

ROUTE=DTYPE-LOWP-FASTPATH-X
REVISION=V001
SOURCE_COMMIT=fccb0d62220214ab8b60fb21668cfd0bb6d6dfab
SOURCE_SHA256=d9297e337f4aee09994ff6d7f41ad3d55ff172a1382ee042b44f791dea114c1c
EXECUTABLE_SHA256=54d687cd50797f3432f24953915e7e5e19c79a9650a9ef549b0451caf72e3e1d
DEVICE=3
CORRECTNESS=INCOMPLETE
BLOCK_REASON=CORRECTNESS_RUNNER_UNAVAILABLE
LOCAL=MEASUREMENT_BLOCKED

The worktree has no existing V001 correctness runner or case matrix. The only newly
created support executable is a host full-link smoke; it passes null device pointers to
`run_kernel`, which returns before an NPU launch, so it cannot establish correctness.
No other route's runner or writable asset was used, no system package was installed or
modified, and the Candidate source was not changed.

Required next action is to provide or authorize an existing compatible correctness
entrypoint/case asset. Since correctness did not PASS, no Local timing was attempted.
