# R5 Paired Backend / Direct Executable Call-Contract Audit

```text
AGENT_ID = W5-R05-VECTOR-ILP-DUALCHAIN-X-ROUTE-AGENT
ROUTE = W5-R05 VECTOR-ILP-DUALCHAIN-X
BRANCH = research/w5-r05-vector-ilp
WORKTREE = /home/data4t2/lelinfeng/cann-w5-r05-vector-ilp
AUDIT_BASE_HEAD = a4dfbf35f6b13955a2d4c74dc66b22e111d43764
RULE_REFRESH = origin/main 1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
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
HISTORICAL_DIRECT_BUILD_FORM = DOCUMENTED, NOT REPRODUCIBLE FROM R5-OWNED FILES
UNIQUE_PAIRED_RUNNER = SHARED-RUNNER CONTRACT ONLY; LAST RECORDED BUILD FAILED
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
