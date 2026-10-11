# R5 Paired Backend / Direct Executable Call-Contract Audit

```text
AGENT_ID = W5-R05-VECTOR-ILP-DUALCHAIN-X-ROUTE-AGENT
ROUTE = W5-R05 VECTOR-ILP-DUALCHAIN-X
BRANCH = research/w5-r05-vector-ilp
WORKTREE = /home/data4t2/lelinfeng/cann-w5-r05-vector-ilp
AUDIT_BASE_HEAD = a4dfbf35f6b13955a2d4c74dc66b22e111d43764
RULE_REFRESH = origin/main 1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
PUBLIC_DIRECT_BUILD_EVIDENCE = origin/main 1c51a5b2
W5_ROUTE_SCOPE = CONTINUE_FIVE_W5_ROUTES / NO_ROUTE_SWITCH
PATCH_SIMILARITY = N/A (documentation-only; no source patch)
MECHANISM_OVERLAP = NO
PERFORMANCE_DONOR = NONE
STAGE = BACKEND_CALL_CONTRACT_AUDIT / BUILD_NOT_ENTERED
HYPOTHESIS = DEPENDENT_VECTOR_CHAIN vs INDEPENDENT_DUAL_VECTOR_CHAIN (unchanged)
```

## Result

```text
ROUTE_OWNED_DIRECT_ADAPTER = NOT_PRESENT
HISTORICAL_R31B_DIRECT_BUILD = PUBLICLY CONFIRMED_BUILD_ONLY / R31B-SOURCE-BOUND
UNIQUE_PAIRED_RUNNER = CURRENT_SHARED-RUNNER_CONTRACT; LAST RECORDED BUILD FAILED
R5_DIRECT_BACKEND_VIA_CURRENT_CONTROL = NO (requires shared backend selector/adapter)
LEGAL_DIRECT_BACKEND = NO
BUILD_THIS_ACTION = NOT_RUN
CORRECTNESS = NOT_RUN
GENERATED_INSTRUCTIONS = NOT_AVAILABLE
LOCAL_RAW = NOT_RUN
SOURCE_SHA_THIS_ACTION = NONE (no source built or edited)
ARTIFACT_SHA_THIS_ACTION = NONE (no artifact produced)
DEVICE_RESOURCE_CHECK = NOT_PERFORMED (no device action)
RESOURCE_STOP_REASON = NONE (this is an interface blocker, not HBM/load/lease)
```

No Vector Candidate was created or edited. The existing R5 orthogonality
receipt remains the complete design contract for the one ILP hypothesis; this
audit changes no kernel mechanism. The earlier V001 failure evidence remains
unchanged. It records an ABI preflight using a frozen accepted source, but
that source was not used as a donor in this audit or as input to any new build.

## Route-owned evidence checked

Only the R5 worktree and R5-owned records/build-output directory were queried.
The checks and results were:

```text
git ls-files 'TOOLING-OWNER-REQUEST.md' \
  '研究/W5-R05-VECTOR-ILP-DUALCHAIN-X/**' \
  '本地实验/W5-R05-VECTOR-ILP-DUALCHAIN-X/**'
RC = 0
RESULT = tracked R5 request, two research receipts, and V001 evidence only;
        no route-owned adapter source or direct paired executable source

find 本地实验/W5-R05-VECTOR-ILP-DUALCHAIN-X \
  研究/W5-R05-VECTOR-ILP-DUALCHAIN-X -type f -printf '%p\t%s bytes\n'
RC = 0
RESULT = V001 evidence plus ignored failed shared-runner CMake outputs;
        no direct executable source, successful executable, or timing raw

rg -n -i 'add_executable|paired_runner|run_kernel_parent|run_kernel_candidate|main\(|backend|w5ctl|argv|runner_identity|aclrtRecordEvent' \
  TOOLING-OWNER-REQUEST.md 本地实验/W5-R05-VECTOR-ILP-DUALCHAIN-X \
  研究/W5-R05-VECTOR-ILP-DUALCHAIN-X
RC = 0
RESULT = only documentation/build-metadata references; no R5-owned direct
         backend dispatch or exact historical CLI/event call implementation

rg -n 'CMAKE_HOME_DIRECTORY|RUNNER_PARENT_SOURCE|RUNNER_CANDIDATE_SOURCE|RUNNER_BUILD_DIR|ASCENDC_ARCH|route_runner' \
  <route-owned V001 build>/CMakeCache.txt \
  <route-owned V001 build>/CMakeFiles/route_runner.dir/link.txt \
  <route-owned V001 build>/CMakeFiles/route_runner_parent.dir/link.txt \
  <route-owned V001 build>/CMakeFiles/route_runner_candidate.dir/link.txt
RC = 0
RESULT = one route_runner linked to the Parent/Candidate shared ASC libraries;
        CMakeCache identifies canonical Tooling source dir, which was not opened

for p in 本地实验/W5-R05-VECTOR-ILP-DUALCHAIN-X/V001/support/parent-runner-probe/build/route_runner \
  本地实验/W5-R05-VECTOR-ILP-DUALCHAIN-X/V001/support/parent-runner-probe/build/libroute_runner_parent.so \
  本地实验/W5-R05-VECTOR-ILP-DUALCHAIN-X/V001/support/parent-runner-probe/build/libroute_runner_candidate.so \
  本地实验/W5-R05-VECTOR-ILP-DUALCHAIN-X/V001/support/parent-runner-probe/build/runner_identity.tsv; do
  test -e "$p" && echo "PRESENT $p" || echo "ABSENT $p"
done
LOOP_RC = 0; each test -e returned 1 / ABSENT
```

The generated R5-owned `link.txt` metadata shows the existing paired contract
builds `parent_adapter.asc.o` into `libroute_runner_parent.so`,
`candidate_adapter.asc.o` into `libroute_runner_candidate.so`, then links both
into one `route_runner` executable. `CMakeCache.txt` records that this failed
build directory was configured from the canonical Tooling source directory;
the source tree itself was not opened or modified for this audit. The existing
request documents the host symbol mapping
`run_kernel` → `run_kernel_parent` / `run_kernel_candidate` and the argument
layout, but the R5 worktree contains no executable or invocation record that
proves how the control layer selects a role, performs correctness, or captures
device-event timing.

The historical direct path is recorded only as a wrapper that includes the
source, macro-renames host entries in namespaces, and uses
`add_executable(paired_runner_v016 paired_runner_v016.asc)`. R5-owned evidence
does not contain that wrapper, its CLI contract, event-timing implementation,
build identity manifest, artifact, or a `w5ctl` backend selector. Therefore its
historical successful build cannot be treated as a current paired backend, and
there is no lawful direct-executable compile/correctness command to run here.

## Public R31B V016 direct-build evidence cross-check

The current public `origin/main` materials named by Main-2 were read without
opening any other Route worktree or reading the included R31B kernel files:

- `本地实验/R31B/V016/support/paired/CMakeLists.txt`
- `本地实验/R31B/V016/support/paired/paired_runner_v016.asc` (only its
  wrapper/include and entry-renaming lines were inspected)
- `本地实验/R31B/V016/support/paired/paired-build-v016-success.txt`

They establish that the successful direct build is source-bound, not a
route-agnostic backend:

1. CMake declares `R31B_V011_SOURCE_DIR` and `R31B_V016_SOURCE_DIR` with
   R31B-specific defaults and builds the standalone target
   `paired_runner_v016` from `paired_runner_v016.asc`.
2. The wrapper literally includes
   `R31B-V011-LP-ROW-PIPELINE_kernel.asc` and
   `R31B-V016-WIDE-TILE-SEED_kernel.asc`, renaming their `run_kernel` entries
   to `run_kernel_v011` and `run_kernel_v016`.
3. The retained success record says `BUILD_RESULT=PASS`, but also
   `BUILD_ACTION=forced compile and link; executable was not started`,
   `DEVICE_CORRECTNESS=NOT_RUN`, and `TIMING=NOT_RUN`.

The CMake source-directory variables being cache-overridable does not by itself
make this a legal R5 backend: the wrapper's include basenames and the produced
standalone target remain fixed, and the success record contains no evidence
that the current unique control layer selects or invokes this executable. R5
could not make it callable through that existing control layer without a
shared Tooling/Integration change to the existing backend selector/adapter
(or equivalent dispatch boundary). Doing so locally would create a separate
execution entry, which is outside the route contract. This audit did not
substitute R5 source under R31B names, modify the wrapper/CMake, read/copy any
R31B kernel source, or use a Champion donor.

Accordingly, the answer to the constrained feasibility question is **NO**:
the public direct build is evidence for a successful R31B-specific executable
build only, not for invoking R5's own source pair from the current sole control
layer without modifying that shared layer. The existing single Owner request
remains the only request; no second request has been created.

## Minimal Tooling/Integration Owner interface patch

This is a route-owned interface request, not a patch applied to shared tooling.
The canonical Tooling/Integration Owner must change the existing paired
control layer in exactly one of these mutually exclusive ways:

1. Keep the existing shared-ASC backend and repair its `__global__ __vector__`
   plugin registration/link contract; or
2. Register the historical direct-executable build as the sole selected
   backend behind that same paired control/timing entry.

For option 2, the smallest semantic delta is: the existing selector chooses
one backend; the backend accepts the existing Parent/Candidate source paths,
expected source SHA-256 values, target architecture, and output directory;
it builds one paired executable exposing the existing role calls and returns
the source/role mapping plus executable SHA-256 to the current driver. The
driver must invoke Parent and Candidate with the same correctness inputs and
the same device-event timing boundary/order already defined by the project
protocol. Do not add a second CLI, timing harness, runner executable, fallback
backend, or identity format. Exact symbol/CLI spelling is intentionally left
to the Owner because the current selector and driver source are outside this
Route's authorized read/write scope.

## Owner and acceptance gate

```text
RESPONSIBLE_OWNER_ROLE = Canonical Tooling/Integration Owner
RESPONSIBLE_OWNER_NAME = NOT IDENTIFIED IN R5-OWNED RECORDS; assignment required
OWNER_ACK = UNCONFIRMED
OWNER_COMMIT = NONE VISIBLE IN R5-OWNED RECORDS
ACCEPTED_BACKEND = NONE
```

Owner acceptance must include a named assignee and commit, identify exactly one
selected backend, and attach a contract-only receipt showing: successful build
and exact source identity; one executable/backend artifact SHA-256; role-call
ABI compatibility; the accepted control-layer correctness invocation; the
unchanged device-event timing boundary; and retained commands/log paths. No
performance result is required for this backend acceptance. Only after it is
accepted may R5 create the dependent/independent Vector source pair, run build
and correctness, inspect generated instructions, then acquire device-event
Local raw under the project resource policy.

Resource rule for subsequent work follows the refreshed public rules: attempt
Compile/Correctness/Local/Profile whenever the selected device has
`FREE_HBM >= 100 MB`. External processes, AICore utilization, high load, and
lease metadata are recorded context, not stop conditions. Stop only for actual
OOM, allocation/runtime resource failure, risk to another user's task, or
server unavailability. No device gate was evaluated in this audit because no
accepted backend was available to invoke.

```text
NEXT_ACTION = WAIT_FOR_NAMED_OWNER_TO_SELECT_AND_ACCEPT_ONE_BACKEND
BLOCKER = NO_ROUTE_OWNED_DIRECT_ADAPTER_OR_VERIFIABLE_PAIRED_CALL_CONTRACT
SECONDARY_BLOCKER = SHARED_TOOLING_BACKEND_SELECTOR_AND_DRIVER_ARE_OUT_OF_R5_WRITE_SCOPE
MICROPROBE_STATUS = NOT_RUN
```

## P4 canonical build/execute adaptation check

```text
P4_CHECK = COMPLETED / READ_ONLY_ROUTE_CONTRACT_EVALUATION
P4_ROUTE_WORKTREE = /home/data4t2/lelinfeng/cann-w5-r05-vector-ilp
P4_ROUTE_HEAD = 8ed086eb3a1df1fe0e59bbee3854d7f2b0f4a52d
P4_CANONICAL_BUILD_ENTRY = /home/data4t2/lelinfeng/cann-tooling-runner/工具/route-runner/route-runner/build_server3.sh
P4_CANONICAL_ENTRY_ACCESS = NOT_OPENED_OR_MODIFIED (dirty shared tooling out of R5 scope)
P4_LEGAL_ROUTE_ADAPTER = NONE
P4_BUILD = NOT_RETRIED (known shared-plugin failure retained)
P4_CORRECTNESS = NOT_RUN
P4_LOCAL = NOT_RUN
P4_KERNEL_REVISION = NONE (contract audit only)
```

The route-owned V001 build metadata gives the exact current input and link
boundary without reopening the shared tooling source. Both ASC targets receive
the caller-supplied source path, SHA-256, `dav-2201`, and a compile definition
that maps `run_kernel` to `run_kernel_parent` or `run_kernel_candidate`:

```text
Parent target = libroute_runner_parent.so from parent_adapter.asc.o
Candidate target = libroute_runner_candidate.so from candidate_adapter.asc.o
Final target = one route_runner linked against both shared libraries
Host ABI = five (GM_ADDR, TensorGroupInfo) pairs, int64_t availableCoreNum,
           aclrtStream, float epsilon
Device entry = templated __global__ __vector__ add_rms_norm_bias_custom<T>
```

The failure occurs after these inputs reach the shared ASC targets and before
any of the three artifacts exists: `Unknown kernelInfo` for the Vector entry,
then unresolved `__origin__add_rms_norm_bias_custom<float/half/__bf16>`.
Therefore the source path, source identity, host role ABI, and final link
shape are not the missing R5-side interface. The missing interface is the
canonical ASC device-entry registration/link contract for a source included
inside the two shared adapter targets.

The published direct-source history is not a currently reusable backend. Its
wrapper/CMake uses fixed R31B include basenames and a standalone
`paired_runner_v016`; its later public Main-1 record shows that direct-source
registration cleared `Unknown kernelInfo` but then failed on missing
`aclrtStream`. It has no accepted canonical selector, source identity record,
or current Parent/Candidate control-layer receipt. R5 therefore did not copy,
rename, invoke, or adapt that history.

The minimum legal shared-owner patch is one change at the existing
`route-runner` registration/selector boundary, with either of these mutually
exclusive implementations:

1. Repair registration/link generation for the existing Parent/Candidate
   shared ASC targets while preserving `RUNNER_*_SOURCE`, expected SHA-256,
   `run_kernel_parent`/`run_kernel_candidate`, and the single `route_runner`;
   or
2. Register one route-agnostic direct backend behind the existing selector,
   consuming those same source paths, SHA-256 values, architecture, and build
   directory, and returning one artifact plus the existing identity record.

The route-owned build metadata identifies the concrete canonical boundary,
without opening its dirty source worktree:

```text
BUILD_ENTRY = /home/data4t2/lelinfeng/cann-tooling-runner/工具/route-runner/route-runner/build_server3.sh
BUILD_ROOT = /home/data4t2/lelinfeng/cann-tooling-runner/工具/route-runner/route-runner
ASC_UNITS = parent_adapter.asc, candidate_adapter.asc
DRIVER_UNIT = runner_main.cpp
TARGETS = route_runner_parent, route_runner_candidate, route_runner
```

For option 1, the patch point is ASC registration and generated-stub/origin
symbol resolution in the two adapter targets; do not alter the source or
weaken either SHA check. For option 2, the patch point is the existing build
entry/selector and its adapter generation: consume the provided absolute paths
and produce the same driver-visible role calls and identity evidence. The
historical Main-1 direct-source attempt additionally shows that a source-TU
adapter must make the `aclrtStream` declaration available (its separate
attempt cleared plugin registration but failed host compilation on that
missing declaration). These are Owner-side shared-tool changes; R5 did not
inspect or edit those files.

The Owner acceptance must name the changed tooling file(s), commit, selected
backend, command, artifact SHA-256, exact source identities, and show: build
RC 0; no `Unknown kernelInfo` or undefined `__origin__` symbols; both role
calls accepted by the existing driver; and unchanged correctness/device-event
timing entry points. This is a contract acceptance only. No R5 Candidate,
second Runner, fallback chain, or new score path is permitted.

```text
P4_DECISION = ADAPTER_NOT_AVAILABLE / FEASIBILITY_UNPROVEN
P4_OWNER = CANONICAL TOOLING/INTEGRATION OWNER (NAME UNCONFIRMED)
P4_OWNER_ACK = UNCONFIRMED
P4_OWNER_COMMIT = NONE
P4_NEXT = named Owner supplies one accepted backend commit and receipt
P4_BLOCKER = shared ASC registration or route-agnostic selector/adapter is not accepted
MICROPROBE = NOT_RUN
```

## Published Canonical Host/backend contract follow-up

```text
PUBLIC_MAIN_REF = origin/main
PUBLIC_MAIN_SHA = 1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f (remote main verified)
PUBLIC_ROUTE_LOCAL_ADAPTER_INPUT = NOT_DOCUMENTED
CANONICAL_BACKEND_SELECTOR_API = NOT_FOUND_IN_PUBLISHED_MAIN
R5_SOURCE_ABI = HOST_ROLE_COMPATIBLE
R5_ROUTE_LOCAL_ADAPTER_CALLABLE_BY_CANONICAL_HOST = NO
```

The published project contract at this revision documents Route ownership,
build/correctness work, and local timing, but exposes no Canonical Host API for
selecting or injecting a route-owned adapter translation unit. The archived
Phase4 `execution-contract.md` is a historical execution policy, not such an
adapter ABI. The published R31B V016 direct build is likewise a fixed-source
standalone executable, not a Canonical Host backend.

The only currently evidenced Canonical build inputs are the R5 V001
`build_server3.sh` contract: `RUNNER_PARENT_SOURCE`,
`RUNNER_CANDIDATE_SOURCE`, expected SHA-256 values, `RUNNER_ASCENDC_ARCH`, and
`RUNNER_BUILD_DIR`. They identify kernel source files and build location; no
published input selects an adapter file or a route-local backend. The
generated V001 build metadata points to these shared units/targets:

```text
/home/data4t2/lelinfeng/cann-tooling-runner/工具/route-runner/route-runner/build_server3.sh
/home/data4t2/lelinfeng/cann-tooling-runner/工具/route-runner/route-runner/parent_adapter.asc
/home/data4t2/lelinfeng/cann-tooling-runner/工具/route-runner/route-runner/candidate_adapter.asc
/home/data4t2/lelinfeng/cann-tooling-runner/工具/route-runner/route-runner/runner_main.cpp
```

The build definitions pass the exact source path and rename only host
`run_kernel` to each role symbol, so the host-call ABI is already represented.
However, the ASC shared-library compile does not register the launched
`__global__ __vector__ add_rms_norm_bias_custom<T>` entry and cannot resolve
the generated `__origin__...` symbols. A shim in the R5 worktree cannot fix
this boundary: the current host has no adapter-path/selector input to consume
it. Supplying the shim as `RUNNER_*_SOURCE` would violate source identity; a
separate CMake target or executable would be a second execution chain.

Therefore the smallest Owner change is to the existing Canonical build
boundary, not to route-local files: keep the source/SHA inputs and existing
driver role ABI, add an accepted route-agnostic source-registration adapter
inside the existing backend selector, and make that path register/link the
Vector device entry while returning the artifact through the same Host
workload. Alternatively, the Owner may repair registration in the current
Parent/Candidate ASC units. No new CLI, Runner, scorer, timing harness, or
fallback chain is part of either patch. The shared tooling worktree was not
opened or modified.

Verification must first use the exact frozen source for both roles and retain
build logs, exact input paths/hashes, selected backend and artifact SHA. Build
must return 0 without `Unknown kernelInfo` or undefined `__origin__` symbols,
and the existing Host must expose both role calls. Only after this acceptance
and an R5 Candidate exists should its correctness run; dependent-versus-
independent timing remains a separate, coordinated measurement stage. No
NPU timing was started in this follow-up.

```text
CANONICAL_HOST_ADAPTATION = NOT_LEGALLY_CALLABLE_FROM_R5_WITHOUT_OWNER_CHANGE
OWNER_REQUEST = EXISTING TOOLING-OWNER-REQUEST.md (no duplicate request)
OWNER_NAME = UNCONFIRMED
OWNER_ACK = UNCONFIRMED
OWNER_COMMIT = NONE
NEXT_CONCRETE_STEP = Owner adds/accepts the adapter registration boundary in the existing Canonical Host
BLOCKER = no published adapter selector/input; ASC Vector device registration/link remains unresolved
KERNEL_REVISION = NONE_ADDED
MICROPROBE = NOT_RUN
LOCAL_SCORE = NONE
```

## Public adapter reuse check and executable minimum plan

```text
PUBLIC_ADAPTER_CHECK = PARTIAL_SHAPE_MATCH / NOT_CALLABLE_BY_CANONICAL_HOST
PUBLIC_W4R01_ABI_MATCH = YES (host role ABI and TensorGroupInfo layout)
PUBLIC_W4R01_SELECTOR_MATCH = NO
PUBLIC_MODE_R015C_ABI_MATCH = NO
ROUTE_LOCAL_ADAPTER_INPUT = NOT_EXPOSED
EXECUTABLE_R5_BACKEND = NONE
```

The published W4-R01 adapter is the closest public shape. Its adapter files
provide `using aclrtStream = void*`, include a `runner_abi.h` containing the
same `TensorInfo`/`TensorGroupInfo` structs, then include one source per role;
its CMake uses two `ascendc_library(... SHARED ...)` targets and one paired
executable. Its public host declarations are the R5-compatible role calls:

```text
extern "C" void run_kernel_parent(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);
extern "C" void run_kernel_candidate(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);
```

That public shape is evidence for the minimum adapter body, not a callable R5
backend. The published W4-R01 CMake and adapter names are route-specific, and
the Canonical Host contract has no input selecting those adapter files. It
also records a build-environment fix (`CPATH`/`C_INCLUDE_PATH` for the HCC
host headers) but does not publish a Canonical selector or artifact handoff
API. R5 cannot copy the route files or create a separate target to consume
them without creating a second execution chain.

The published MODE-X adapter is not ABI-compatible: its exported functions
are `mode_r015c_parent_launch(uint32_t, void*, void*, void*, aclrtStream)` and
`mode_r015c_candidate_launch(...)`, with route-specific tiling headers, fixed
source/tiling SHA values, and a standalone `r015c_pair_runner`. It cannot
stand in for R5's five `(GM_ADDR, TensorGroupInfo)` pairs, core count, and
epsilon role ABI.

The legal minimum plan, once an Owner change is accepted, is therefore:

```text
1. Canonical selector accepts one route-local adapter registration for the
   caller's Parent/Candidate source paths and expected SHA-256 values.
2. Each selected adapter supplies the public-compatible ABI shim
   (TensorInfo/TensorGroupInfo plus aclrtStream declaration), includes exactly
   its selected source, and maps run_kernel to the existing role symbol.
3. The existing ASC registration/build boundary emits a linkable device stub
   for add_rms_norm_bias_custom<float/half/__bf16>; no __origin__ symbol is
   left unresolved.
4. The same single route_runner/Host workload returns the two role artifacts
   and identity record; no new CLI, runner, scorer, or timing chain is added.
5. R5 then runs the supplied canonical correctness command, checks generated
   instructions, and only after a coordinated free timing slot runs the
   dependent/independent pair with identical shape, dtype, memory traffic,
   synchronization, warmup, event boundary, and sample protocol.
```

The exact missing interface is not a missing R5 host function: the current
`runner_main.cpp` role signature is already represented in V001 metadata. It
is the absent adapter-selection input and the device registration boundary
between these existing Canonical files:

```text
/home/data4t2/lelinfeng/cann-tooling-runner/工具/route-runner/route-runner/build_server3.sh
/home/data4t2/lelinfeng/cann-tooling-runner/工具/route-runner/route-runner/parent_adapter.asc
/home/data4t2/lelinfeng/cann-tooling-runner/工具/route-runner/route-runner/candidate_adapter.asc
/home/data4t2/lelinfeng/cann-tooling-runner/工具/route-runner/route-runner/runner_main.cpp
```

The Owner must expose one adapter-selection/source-registration hook at that
boundary, preserve the existing role ABI and source identity checks, and make
the generated `__global__ __vector__` registration resolve the corresponding
`__origin__` symbols. The source and route-local files do not need a new host
function. The public W4-R01 shim (`aclrtStream` declaration plus ABI header)
is the compatibility pattern the Owner may use, not a permission for R5 to
modify shared tooling.

The only known build command remains the existing canonical entry, staged for
use only after Owner acceptance and after R5 has a real Parent/Candidate pair:

```bash
RUNNER_PARENT_SOURCE=<R5-parent-source> \
RUNNER_CANDIDATE_SOURCE=<R5-independent-or-dependent-source> \
RUNNER_PARENT_SHA256=<sha256> \
RUNNER_CANDIDATE_SHA256=<sha256> \
RUNNER_ASCENDC_ARCH=dav-2201 \
RUNNER_BUILD_DIR=/home/data4t2/lelinfeng/cann-w5-r05-vector-ilp/本地实验/W5-R05-VECTOR-ILP-DUALCHAIN-X/V001/accepted-build \
bash /home/data4t2/lelinfeng/cann-tooling-runner/工具/route-runner/route-runner/build_server3.sh
```

This command was not executed in the follow-up: the adapter selector, Owner
acceptance, R5 Candidate source pair, and canonical correctness invocation
are absent. Reusing the same command with the frozen source would repeat the
known failure and would not establish an R5 Candidate. No timing or score is
claimed.

```text
PUBLIC_ADAPTER_DECISION = NOT_ALLOWED_AS_CURRENT_CANONICAL_BACKEND
MINIMUM_OWNER_CHANGE = selector/source-registration hook + public-compatible ABI shim + device-stub/origin link fix
MISSING_HOST_FUNCTION = NONE_IDENTIFIED
MISSING_SELECTOR_INTERFACE = route-local adapter/source registration input
MISSING_REGISTRATION_POINT = parent_adapter.asc/candidate_adapter.asc ASC device-entry registration boundary
OWNER_FILES = build_server3.sh, parent_adapter.asc, candidate_adapter.asc, runner_main.cpp (shared tooling scope)
R5_BUILD = NOT_RUN
R5_CORRECTNESS = NOT_RUN
R5_MICROPROBE = NOT_RUN
R5_LOCAL_SCORE = NONE
```
