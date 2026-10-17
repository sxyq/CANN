# R5 Direct-Executable Backend Adapter Feasibility

```text
AGENT_ID = W5-R05-VECTOR-ILP-DUALCHAIN-X-ROUTE-AGENT
ROUTE = W5-R05 VECTOR-ILP-DUALCHAIN-X
WORKTREE = /home/data4t2/lelinfeng/cann-w5-r05-vector-ilp
BRANCH = research/w5-r05-vector-ilp
DATE = 2026-10-10
EVIDENCE_HEAD = 2899044d80c556aa316342e6c75f1f6cac62e537
```

## Decision

```text
R5_LEGAL_BACKEND_ADAPTER = CONDITIONAL_ONLY / NOT_CURRENTLY_LEGAL
BACKEND_ACCEPTANCE_SLOT = ONE
ACTIVE_BACKEND = NONE
OWNER = CANONICAL TOOLING OWNER (NAME UNCONFIRMED)
OWNER_ACK = UNCONFIRMED
MICROPROBE_STATUS = NOT_RUN
```

The historical direct-executable path is technically adaptable in principle,
but it is not a legal `w5ctl` backend on the evidence currently available. Its
historical success proves only that a wrapper containing the source directly
could be built as one executable. It does not prove registration in the
current `w5ctl` backend selection contract, source identity propagation, or
the current Parent/Candidate control and timing interface.

This is not a second request or a second execution chain. The existing
`TOOLING-OWNER-REQUEST.md` remains the only R5 tooling request. The direct
backend is an alternative implementation of the one pending acceptance slot:
the Owner must select and accept either the shared-ASC registration repair or
the direct-executable adapter. R5 must not run or accept both paths.

## Historical evidence

The recorded direct path has these properties:

1. The wrapper included the source directly and macro-renamed the host entry
   points in namespaces.
2. CMake used `add_executable(paired_runner_v016 paired_runner_v016.asc)`.
3. One direct executable was linked with the runtime libraries.

The current canonical path has a different contract: separate Parent and
Candidate adapters are built as shared ASC libraries and a third
`route_runner` executable links them. The route evidence records the shared
path failure as `Unknown kernelInfo` followed by undefined
`__origin__add_rms_norm_bias_custom<...>` symbols. Therefore the historical
direct path cannot be substituted locally or treated as an already accepted
Runner.

Sources:

- `TOOLING-OWNER-REQUEST.md`, Contract difference from the historical
  successful path.
- `本地实验/W5-R05-VECTOR-ILP-DUALCHAIN-X/V001/compile-evidence.md`, valid
  build attempt and retained failure.
- `研究/W5-R05-VECTOR-ILP-DUALCHAIN-X/ORTHOGONALITY-RECEIPT.md`, controlled
  experiment requirement for one existing harness/runner and one ABI.

The frozen accepted source named by the existing request may be used by the
Tooling Owner as an ABI/identity acceptance fixture only. It is not a R5
Candidate, donor, performance baseline, or Champion migration source in this
receipt.

## Minimum Tooling Owner interface

The following is the minimum semantic interface that the canonical Tooling
Owner must expose and accept. It is not permission for R5 to implement it in
the tooling worktree, and it does not prescribe unobserved symbol names.

1. Register one direct-executable backend in the existing `w5ctl` backend
   registry/selector. The selector must choose exactly one backend for an R5
   run; no shared-Runner fallback or parallel direct path may be active.
2. Accept the existing Parent and Candidate source inputs, expected SHA-256
   values, `dav-2201` target, and route-owned build directory without rewriting
   source bytes or substituting an untracked file.
3. Preserve the existing host-call semantics for
   `run_kernel_parent`/`run_kernel_candidate`, including the five GM
   arguments, five `TensorGroupInfo` arguments, core count, stream, and
   epsilon. A direct translation unit may implement these symbols differently,
   but the `w5ctl` observable ABI must remain unchanged.
4. Return one executable backend artifact plus a machine-readable identity
   record containing the selected backend, source paths and SHA-256 values,
   build command, target architecture, and artifact SHA-256. The record must
   be consumable by the existing correctness/timing control layer.
5. Keep correctness, generated-instruction inspection, and timing as separate
   stages. Backend acceptance must not silently run a microprobe or create a
   Candidate.

## Rules and acceptance conditions

The route constraints require one route/worktree/branch/context, no
route-created shared tooling change, exact source identity, and the existing
harness/ABI for both variants. The local timing protocol also requires
same-binary qualification before interleaved Parent/Candidate timing. These
rules make a historical standalone executable insufficient until it is
wrapped by the canonical control-layer contract.

An Owner acceptance receipt must identify the Owner commit and selected backend
and demonstrate all of the following with one contract-only fixture:

1. `w5ctl` logs exactly one selected backend and no fallback or second Runner.
2. The exact request inputs build successfully with return code 0.
3. The direct executable artifact and identity record exist; source paths and
   SHA-256 values match the supplied values byte-for-byte.
4. The selected backend exposes the existing host ABI and can represent both
   Parent and Candidate roles without changing the source or creating a second
   execution chain.
5. The selected build log has no `Unknown kernelInfo`, undefined
   `__origin__add_rms_norm_bias_custom<...>`, source substitution, or identity
   mismatch.
6. The receipt names the exact Tooling Owner, tooling commit, backend selector
   entry, acceptance command, artifact SHA-256, and retained log paths.

This acceptance is a build/identity contract result, not a performance result.
Only after it is accepted may R5 create the two equal-compute Vector source
variants, verify Parent identity/correctness and generated instructions, and
then consider the planned microprobe through the one selected backend.

## Current blocker and next action

```text
BLOCKER = OWNER_ACK_UNCONFIRMED
SECONDARY_BLOCKER = W5CTL_BACKEND_REGISTRY_AND_ADAPTER_CONTRACT_NOT_EXPOSED_IN_R5_RECORDS
PERMISSION_BLOCKER = R5_CANNOT_MODIFY_OR_VALIDATE_THE_CANONICAL_DIRTY_TOOLING_WORKTREE
CANDIDATE = NOT_CREATED
RUNNER = NOT_RUN
MICROPROBE = NOT_RUN
PERFORMANCE_INFERENCE = NONE
```

The next action is to wait for one named Tooling Owner to provide one accepted
backend implementation and receipt. Until then, R5 remains
`OWNER_FIX_WAIT / FEASIBILITY_UNPROVEN`; neither the historical direct path nor
the shared-Runner repair is an executable backend for this route.
