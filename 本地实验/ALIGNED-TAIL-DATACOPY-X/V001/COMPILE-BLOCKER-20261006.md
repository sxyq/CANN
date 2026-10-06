# ALIGNED-TAIL-DATACOPY-X V001 — Compile Blocker

- Observed at: `2026-10-06T22:01:34Z`
- Worktree: `/home/data4t2/lelinfeng/cann-r-w4-5-aligned-tail`
- Branch / HEAD: `route/r-w4-5-aligned-tail-datacopy-x` / `d8c3b380276df5549f057dee19031e04fdd874a9`
- Candidate: `本地实验/ALIGNED-TAIL-DATACOPY-X/V001/submission.asc`
- Candidate size: 190020 bytes, 3554 lines
- Candidate SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Stage: `COMPILE=NOT_RUN` (no compile/link command was issued)

## Blocker

The V001 route directory contains only `submission.asc`. It has no exact route-owned
`CMakeLists.txt`, build/compile script, host/device wrapper, correctness runner,
or parent runner. The source documents that it is included by a direct-invocation
runner and relies on runner-provided `TensorGroupInfo` / `TensorInfo` definitions.
The repository environment check reported no built canonical runner artifact.
Without the matching wrapper and runner contract, a compile/link result or runtime
correctness result cannot be produced or attributed to this exact Candidate.

## Environment context

- CANN: `8.5.T8.0.B060`; `bisheng`, `ccec`, CMake, and `libascendcl.so` available
  after sourcing `/usr/local/Ascend/ascend-toolkit/set_env.sh`.
- SoC / architecture: `dav-2201` / `NpuArch=2201`.
- Assigned Local device: `4`; environment support reports devices 0–6 with
  approximately 5.2–6.3 GB FREE_HBM and VLLM load, and device 7 with approximately
  40.7 GB FREE_HBM plus Python/AICore load. These are context, not admission gates.
- No NPU command was run for this blocker record; no process was stopped.

## Next action

Obtain or restore the exact runner/build assets for this route in its own worktree.
Then compile/link the unchanged V001 source. On Compile PASS, immediately run
Correctness; on Correctness PASS, immediately run Local on device 4 and record
the observed FREE_HBM/load. No Online action is authorized.
