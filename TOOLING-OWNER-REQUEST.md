# Tooling Owner Request: R5 Existing Runner Registration Compatibility

```text
ROUTE = W5-R05 VECTOR-ILP-DUALCHAIN-X
AGENT_ID = W5-R05-VECTOR-ILP-DUALCHAIN-X-ROUTE-AGENT
BRANCH = research/w5-r05-vector-ilp
REQUEST_STATUS = REQUEST_PUBLISHED / OWNER_ACK_UNCONFIRMED
REQUEST_PUBLICATION_COMMIT = e4c49a15932f30727d6a83d9f4134eff4e0216c4
REQUEST_DELIVERY_CHANNEL = R5 route-owned request file pushed to origin/research/w5-r05-vector-ilp
R5_TOOLING_OWNER = CANONICAL TOOLING OWNER (NAME UNCONFIRMED)
OWNER_ACK = UNCONFIRMED (no Owner commit or acceptance receipt)
ORIGINAL_BLOCKER = EXISTING_CANONICAL_RUNNER_CANNOT_REGISTER_THE_FROZEN_KERNEL
ORIGINAL_BLOCKER_REVISION = 4442c95e
REQUEST_SCOPE = ONE_EXISTING_BACKEND_SELECTOR_ADAPTER_CHANGE; NO_SECOND_REQUEST
RESPONSIBLE_OWNER_ROLE = CANONICAL TOOLING/INTEGRATION OWNER
RESPONSIBLE_OWNER_NAME = UNCONFIRMED / MUST_BE_NAMED_IN_ACK
SELECTED_BACKEND = NONE
```

## Delivery and acknowledgement status

The pushed route-owned request file is the only delivery evidence available to
R05. No named Tooling Owner, direct owner endpoint, acknowledgement, owner
commit, or acceptance receipt is present in the route-owned records. Publication
of this request is not acceptance and does not authorize use of uncommitted
tooling changes.

## Request

Please repair the existing canonical `route-runner` source-registration path so
it can build the frozen accepted source below through the existing single
Runner. Do not create a second Runner, a second execution chain, or a new route
worktree. Do not change the source bytes or source identity.

```text
SOURCE = /home/data4t2/lelinfeng/cann-w5-r05-vector-ilp/线上结果/R31B/V011/submission.asc
SOURCE_SHA256 = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TOOLING = /home/data4t2/lelinfeng/cann-tooling-runner/工具/route-runner/route-runner/build_server3.sh
CANN = 8.5.T8.0.B060
ASCENDC_ARCH = dav-2201
FIXTURE_ROLE = CONTRACT_ONLY; NOT_R5_DONOR; NOT_CANDIDATE; NO_PERFORMANCE_USE
```

## Confirmed compatibility boundary

The source and current Runner host ABI are compatible:

- The source exports `extern "C" void run_kernel(...)` with five GM
  arguments, five `TensorGroupInfo` arguments, `int64_t availableCoreNum`,
  `aclrtStream`, and `float epsilon`.
- The current adapters include the source directly and rename `run_kernel` to
  `run_kernel_parent` or `run_kernel_candidate` through the existing compile
  definitions.
- `runner_main.cpp` declares and calls those two renamed functions with the
  same argument layout.
- The preflight used the exact accepted source as both Parent and Candidate;
  absolute paths, source SHA-256 values, and `dav-2201` reached both ASC
  wrapper targets.

The device entry is the source's existing templated entry:

```text
template <typename T>
__global__ __vector__ void add_rms_norm_bias_custom(...)
```

The host function launches its existing `float`, `half`, and `bfloat16_t`
specializations. This request is for registration/build compatibility only;
it does not request any kernel edit or performance change.

## Current blocking failure

The current Runner compiles the adapters as two `ascendc_library(... SHARED ...)`
targets and then links `route_runner`. With the identical frozen source on
both sides, ASC plugin registration fails before a shared library or executable
is produced:

```text
[ERROR] ASCPLUGIN ... Unknown kernelInfo,
  mangledName: _Z24add_rms_norm_bias_customPhS_S_S_S_mmjff
ld.lld: error: undefined symbol:
  void __origin__add_rms_norm_bias_custom<float>(...)
ld.lld: error: undefined symbol:
  void __origin__add_rms_norm_bias_custom<half>(...)
ld.lld: error: undefined symbol:
  void __origin__add_rms_norm_bias_custom<__bf16>(...)
cceld: ccec ReturnCode: 1
```

The retained route evidence is:

```text
/home/data4t2/lelinfeng/cann-w5-r05-vector-ilp/本地实验/W5-R05-VECTOR-ILP-DUALCHAIN-X/V001/compile-evidence.md
```

The tooling-owned fixture also reproduced `Unknown kernelInfo` under the same
shared registration path, so this is a current ASC plugin/shared-target
compatibility blocker rather than an R05 Candidate or timing result.

## Contract difference from the historical successful path

The historical R31B paired path used a single direct executable translation
unit:

- the wrapper included the source directly, with macro-renamed host entry
  points in namespaces;
- CMake used `add_executable(paired_runner_v016 paired_runner_v016.asc)`;
- the result was one direct executable linked with the runtime libraries.

The current canonical Runner instead includes each source through a separate
adapter, builds `route_runner_parent` and `route_runner_candidate` as shared
ASC libraries, and links a third `route_runner` executable against them. The
historical path therefore does not prove that the current shared ASC plugin
registration contract accepts the source. Reusing that path would violate the
single-Runner requirement and is explicitly out of scope.

## Precise selector/adapter request from the public V016 contract

The public V016 materials establish a successful **R31B-source-bound build**,
not a reusable R5 backend: its CMake uses R31B-specific source directories,
its wrapper includes fixed R31B kernel basenames, and its target is the
standalone `paired_runner_v016`. The success receipt also records compile/link
only; correctness and timing were not run. R5 must not copy that wrapper,
rename R5 files to its include names, use R31B kernel bytes as a donor, or
start that standalone target.

The single existing control layer needs one Owner-side selector/adapter change
with this minimum semantic interface:

1. The existing selector chooses exactly one backend for a run and reports its
   backend identity. The direct backend, if selected, is an implementation
   behind the existing control/timing entry; it is not a second CLI, Runner,
   score evaluator, fallback chain, or active execution path.
2. The selected adapter consumes the existing caller-supplied
   `RUNNER_PARENT_SOURCE`, `RUNNER_CANDIDATE_SOURCE`, their expected SHA-256
   values, `RUNNER_ASCENDC_ARCH`, and `RUNNER_BUILD_DIR`. It must not replace
   these with `R31B_V011_SOURCE_DIR`, `R31B_V016_SOURCE_DIR`, fixed R31B include
   basenames, or an untracked source copy.
3. The adapter preserves the existing role ABI and control semantics:
   `run_kernel_parent` and `run_kernel_candidate`, each with five GM
   arguments, five `TensorGroupInfo` arguments, `int64_t availableCoreNum`,
   `aclrtStream`, and `float epsilon`. A direct translation unit may implement
   the internal symbol mapping, but the existing control layer must call both
   roles through the same interface.
4. The adapter returns the one selected backend artifact through the existing
   control layer and extends the existing identity record, if needed, with
   backend name, exact source paths/SHA-256 values, target architecture, build
   command, and artifact SHA-256. It must not invent a parallel identity format.
5. Correctness, generated-instruction inspection, and device-event timing stay
   separate stages. Backend selection/build must not create a Vector Candidate,
   start a microprobe, or change the ILP experiment's dtype, operation count,
   memory traffic, synchronization, shape, or timing boundary.

This is one request to the existing Tooling/Integration Owner. It asks for a
route-agnostic adapter behind the unique control layer; it does not authorize
R5 to modify shared tooling. The Owner must choose either this direct adapter
or the shared-ASC registration repair and provide one acceptance receipt, not
both.

## Minimal repair scope

Change only the existing canonical `route-runner` registration/build boundary
so the accepted source's generated `__global__ __vector__` entry is registered
and linkable in the current Parent/Candidate shared targets. Preserve all of
the following:

1. The existing `RUNNER_PARENT_SOURCE` and `RUNNER_CANDIDATE_SOURCE` inputs.
2. The existing SHA-256 identity checks and `runner_identity.tsv` output.
3. The existing `run_kernel_parent` / `run_kernel_candidate` host ABI.
4. The existing single `route_runner` executable and its timing/correctness
   interface.
5. No source rewrite, no Candidate edit, no second Runner, and no direct
   executable fallback chain.

## Acceptance conditions

Using the exact preflight command recorded in the route evidence, with the
frozen source supplied for both Parent and Candidate:

1. `build_server3.sh` exits with status 0.
2. All three artifacts exist: `route_runner`,
   `libroute_runner_parent.so`, and `libroute_runner_candidate.so`.
3. The build log contains neither `Unknown kernelInfo` nor an
   `__origin__add_rms_norm_bias_custom` undefined-symbol error.
4. `runner_identity.tsv` records the exact source paths and
   `SOURCE_SHA256` shown above without substitution.
5. The accepted source can then be handed back to R05 for separate correctness
   and generated-instruction checks before any ILP microprobe.

For the direct-adapter option, additionally require that the acceptance log
shows the selector choosing exactly one backend while consuming caller-supplied
source paths rather than fixed R31B paths, and that the adapter exposes the
same Parent/Candidate role calls to the existing driver. No R5 Candidate or
R31B kernel donor is needed or permitted for this contract request.

The final acceptance step must not be interpreted as a performance result.
Until it passes, R05 remains `FEASIBILITY_UNPROVEN`; correctness, generated
instructions, timing, score, Online submission, and Champion migration remain
`NOT_RUN`/`NO`.

## Route disposition while blocked

```text
CLASSIFICATION = FEASIBILITY_UNPROVEN
BLOCKER = EXISTING_CANONICAL_RUNNER_CANNOT_REGISTER_THE_FROZEN_KERNEL
ADAPTER_BLOCKER = SHARED_BACKEND_SELECTOR_AND_ROUTE_AGNOSTIC_ADAPTER_NOT_ACCEPTED
OWNER = CANONICAL TOOLING/INTEGRATION OWNER (NAME UNCONFIRMED)
OWNER_COMMIT = NONE
ACCEPTED_BACKEND = NONE
CANDIDATE_EDIT = NONE
MICROPROBE = NOT_RUN
PERFORMANCE_INFERENCE = NONE
ONLINE = NO
CHAMPION_MIGRATION = NO
```
