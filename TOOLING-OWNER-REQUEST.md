# Tooling Owner Request: R5 Existing Runner Registration Compatibility

```text
ROUTE = W5-R05 VECTOR-ILP-DUALCHAIN-X
AGENT_ID = W5-R05-VECTOR-ILP-DUALCHAIN-X-ROUTE-AGENT
BRANCH = research/w5-r05-vector-ilp
REQUEST_STATUS = REQUEST_ONLY / NOT_SENT
ORIGINAL_BLOCKER = EXISTING_CANONICAL_RUNNER_CANNOT_REGISTER_THE_FROZEN_KERNEL
ORIGINAL_BLOCKER_REVISION = 4442c95e
```

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

The final acceptance step must not be interpreted as a performance result.
Until it passes, R05 remains `FEASIBILITY_UNPROVEN`; correctness, generated
instructions, timing, score, Online submission, and Champion migration remain
`NOT_RUN`/`NO`.

## Route disposition while blocked

```text
CLASSIFICATION = FEASIBILITY_UNPROVEN
BLOCKER = EXISTING_CANONICAL_RUNNER_CANNOT_REGISTER_THE_FROZEN_KERNEL
CANDIDATE_EDIT = NONE
MICROPROBE = NOT_RUN
PERFORMANCE_INFERENCE = NONE
ONLINE = NO
CHAMPION_MIGRATION = NO
```
