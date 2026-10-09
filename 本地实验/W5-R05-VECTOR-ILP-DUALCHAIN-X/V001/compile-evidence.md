# W5-R05 V001 Feasibility Preflight

```text
AGENT_ID = W5-R05-VECTOR-ILP-DUALCHAIN-X-ROUTE-AGENT
ROUTE = W5-R05 VECTOR-ILP-DUALCHAIN-X
REVISION = V001 (harness/source-contract preflight; no candidate edit)
WORKTREE = /home/data4t2/lelinfeng/cann-w5-r05-vector-ilp
BRANCH = research/w5-r05-vector-ilp
HOST = hwnput3
USER = lelinfeng
SOC = Ascend910B3 / dav-2201
CANN = Version=8.5.T8.0.B060 (version_dir=8.5.0.alpha002)
```

## Existing runner used

The only runner invoked was the existing project runner at:

```text
/home/data4t2/lelinfeng/cann-tooling-runner/工具/route-runner/route-runner/build_server3.sh
```

That tooling worktree was not modified. The build output was directed to the
route-owned ignored evidence directory:

```text
/home/data4t2/lelinfeng/cann-w5-r05-vector-ilp/本地实验/W5-R05-VECTOR-ILP-DUALCHAIN-X/V001/support/parent-runner-probe/build
```

For this preflight only, Parent and Candidate both pointed at the frozen
Champion source. This tests the runner/source ABI without claiming a route
Candidate or a performance comparison:

```text
PARENT = 线上结果/R31B/V011/submission.asc
PARENT_SHA256 = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE = same Parent source for ABI preflight only
```

The first invocation with `RUNNER_ASCENDC_ARCH=dav-c220` failed immediately
with the compiler's unsupported-architecture diagnostic. The same build
directory was then retried with the installed 910B3 spelling `dav-2201`; no
second runner or execution chain was created.

## Valid build attempt

```bash
RUNNER_PARENT_SOURCE=/home/data4t2/lelinfeng/cann-w5-r05-vector-ilp/线上结果/R31B/V011/submission.asc \
RUNNER_CANDIDATE_SOURCE=/home/data4t2/lelinfeng/cann-w5-r05-vector-ilp/线上结果/R31B/V011/submission.asc \
RUNNER_PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3 \
RUNNER_CANDIDATE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3 \
RUNNER_ASCENDC_ARCH=dav-2201 \
RUNNER_BUILD_DIR=/home/data4t2/lelinfeng/cann-w5-r05-vector-ilp/本地实验/W5-R05-VECTOR-ILP-DUALCHAIN-X/V001/support/parent-runner-probe/build \
bash /home/data4t2/lelinfeng/cann-tooling-runner/工具/route-runner/route-runner/build_server3.sh
```

The compiler entered both ASC wrapper targets, then failed before producing a
device binary or shared library:

```text
[ERROR] ASCPLUGIN ... Unknown kernelInfo, mangledName: _Z24add_rms_norm_bias_customPhS_S_S_S_mmjff
.../线上结果/R31B/V011/submission.asc:3469, col:28
ld.lld: error: undefined symbol: void __origin__add_rms_norm_bias_custom<float>(...)
ld.lld: error: undefined symbol: void __origin__add_rms_norm_bias_custom<half>(...)
ld.lld: error: undefined symbol: void __origin__add_rms_norm_bias_custom<__bf16>(...)
cceld: ccec ReturnCode: 1
gmake: *** [Makefile:136: all] Error 2
```

There is no `route_runner`, parent shared library, candidate shared library,
device binary, or generated-instruction listing in the build output. The
failure is the existing runner's ASC plugin/source registration contract, not
an NPU resource admission failure.

## Device context

`npu-smi info` on `hwnput3` reported eight healthy `910B3` devices. Raw HBM
usage was `33750/65536`, `33962/65536`, `3426/65536`, `3428/65536`,
`59194/65536`, `58515/65536`, `58513/65536`, and `24790/65536` pages for
devices 0 through 7 respectively. Every device retained more than 100 MB
free by the project's admission rule. Existing VLLM/user processes were left
untouched. No device execution was attempted after the compile failure.

## Result

```text
COMPILE = FAIL (existing runner plugin registration / link)
CORRECTNESS = NOT_RUN
GENERATED_INSTRUCTIONS = NOT_AVAILABLE
LOCAL_MEASUREMENT = NOT_RUN
LOCAL_SCORE = NONE
LOCAL_DELTA = NONE
CLASSIFICATION = FEASIBILITY_UNPROVEN
ONLINE = NO
CHAMPION_MIGRATION = NO
BLOCKER = EXISTING_CANONICAL_RUNNER_CANNOT_REGISTER_THE_FROZEN_KERNEL
```

The dependent and independent Vector source pair was not created because the
existing runner could not first accept even the identical frozen source. No
timing, score, or mechanism conclusion is inferred. The next action is to use
the same existing runner after its accepted source/ABI registration contract is
available; until then this route remains `FEASIBILITY_UNPROVEN`.
