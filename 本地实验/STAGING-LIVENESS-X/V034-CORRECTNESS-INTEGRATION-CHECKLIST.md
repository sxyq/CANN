# STAGING-LIVENESS-X V034 Correctness Integration Checklist

Status: prepared from the existing V034 `run_kernel` ABI only. This is a route-local handoff checklist, not a runner implementation.

## Frozen inputs

- Candidate: `本地实验/STAGING-LIVENESS-X/V034/src/submission.asc`
- Candidate SHA-256: `75131108d32bc587c3c289d53ded04a15c5224417c675cb37a46df032e104d33`
- Parent: V033, SHA-256 `0df982248c80c4f2e5541dc730b0a25ba97544efc3b3e3e3409c24deed2d2eff`
- The V034 root and `src/submission.asc` copies currently have the same candidate SHA.
- V034 stays frozen. Do not edit either candidate copy, create V035, or perform Online work.
- Existing compile evidence covers the `device` and `submission` object targets only; `full_link` was not run. Its current adapter `main` is a no-op and is not correctness evidence.

## Support handoff gate

Do not run correctness until Support supplies the proven generic-runner commit and exact CLI. Record these verbatim from that handoff; do not infer flags or build a route-specific runner:

```text
SUPPORT_COMMIT=<full commit SHA>
PROVEN_CLI=<exact command line>
RUNNER_SOURCE_OR_ENTRY=<path named by Support>
CANDIDATE_INPUT=<exact V034 source path/argument accepted by that CLI>
TEST_SET=T01-T15
SUPPORT_VALIDATION_EVIDENCE=<reference supplied with the proven handoff>
```

Consume the supplied runner/CLI as documented. Keep any generic runner repair in its tooling worktree; do not copy it into V034 or create an alternate harness.

## ABI contract

The runner must invoke the frozen candidate through this existing C ABI, preserving parameter order and types:

```cpp
extern "C" void run_kernel(
    GM_ADDR x,
    const TensorGroupInfo& info_x,
    GM_ADDR residual,
    const TensorGroupInfo& info_residual,
    GM_ADDR gamma,
    const TensorGroupInfo& info_gamma,
    GM_ADDR bias,
    const TensorGroupInfo& info_bias,
    GM_ADDR output,
    const TensorGroupInfo& info_output,
    int64_t availableCoreNum,
    aclrtStream stream,
    float epsilon);
```

The metadata declarations already used by the V034 adapter are:

```cpp
struct TensorInfo {
    const int64_t* shape;
    int64_t numDims;
    int32_t dtype;
};

struct TensorGroupInfo {
    const TensorInfo* tensors;
    int64_t numTensors;
};
```

The ten arguments before `availableCoreNum` are five GM addresses interleaved with their corresponding `TensorGroupInfo` references, in the order `x`, `residual`, `gamma`, `bias`, `output`. The final arguments are `availableCoreNum`, stream, then `epsilon`.

## Route execution sequence

1. Verify the supplied commit and CLI match the Support handoff; preserve the exact command and its working directory in the run record.
2. Bind the runner to the V034 source above. Before execution, verify the selected source SHA is the frozen candidate SHA; do not silently fall back to another source or binary.
3. Build/link only through the proven CLI. Save its complete output, exit status, produced executable identity if exposed, and the candidate SHA it consumed. This closes the current object-only compile gap; it does not change V034's kernel source.
4. Run all correctness cases T01-T15 with the supplied runner. Preserve per-case outcome, numerical comparison summary, full command/output, and return code in new V034 evidence files without overwriting existing evidence.
5. Mark V034 Correctness PASS only if T01-T15 all complete against the frozen candidate source and every case passes the runner's stated numerical checks. A runner/setup failure is incomplete, not a kernel failure; a completed numerical mismatch is a correctness failure. In either case, do not start Local unless Correctness is PASS.
6. After Correctness PASS, run the assigned numeric Local comparison for direct Parent V033 versus Candidate V034 using the proven runner and identical inputs, timing boundary, and device conditions. Use the assigned device/protocol, interleave Parent/Candidate samples, and retain raw samples and load snapshots.
7. Record the resulting Local status and evidence separately from Correctness. Online remains forbidden.

## Current stop point

Support's proven commit/CLI has not been supplied in this checklist. Until that handoff arrives, keep V034 frozen and stop before runner integration, Correctness, and Local.

## Superseding Planning execution update (2026-10-07)

The latest Planning directive explicitly authorizes route-local build/wrapper/runner/harness support and supersedes the Support-commit gate above. The exact V034 direct-invocation runner is built and bound to the frozen source SHA; Parent V033 and Candidate V034 have both completed the route-local T01-T15 suite. Both fail only T14 FP32 `[2,1,2,32768]`; details and raw evidence are in `V034/support/results/correctness-pair-20261007T231845Z/`. Candidate Local was not run because its Correctness gate failed. Keep the candidate frozen and await Planning direction; no V035 or shared-record changes were made.
