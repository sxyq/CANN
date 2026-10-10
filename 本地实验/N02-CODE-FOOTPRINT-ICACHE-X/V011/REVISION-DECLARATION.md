# N02 V011 Revision Declaration

ROUTE=N02 CODE-FOOTPRINT-ICACHE-X
REVISION=V011
PARENT_REVISION=V010
PARENT_COMMIT=c283937370adbefb60622f75656af5beadadf092
RULE_REFRESH_RECEIPT=ROUTE=N02 CODE-FOOTPRINT-ICACHE-X; HEAD=c283937370adbefb60622f75656af5beadadf092; PARENT=3035fa9afe922950725aecd6fe8616e62ba07df5; MECHANISM=executable hotspot footprint and instruction-fetch layout; FORBIDDEN=R029 runtime dispatch, repeated noinline, equivalent function reordering, section metadata only, N01 FP32 lifetime, N03 Vector issue, N04 GM/L2, N05 Scalar address, new Runner, new scheduler, new ledger

## Single Source Change

V011 starts from the real V008 source and changes one declaration only:

```diff
- __aicore__ inline void ProcessFp16FullRowOutputPipelined(...)
+ __aicore__ __attribute__((always_inline)) inline void ProcessFp16FullRowOutputPipelined(...)
```

The function is called by `Process()` for the existing FP16
`rowWidth == kCacheElems && localRows > 1` path. Its body, call arguments,
shape predicates, arithmetic, DMA calls, event operations and synchronization
calls are unchanged. This is a compiler-supported call-layout factor and is
independent of the stopped noinline and source-definition-order variants.

SOURCE_PARENT_COMMIT=ca28605bcf1fed9a9c43706f881d6c322f902c75
SOURCE_PARENT_PATH=V008/submission.asc
SOURCE_PARENT_SHA256=4a2788e57efe4a0019a68928fec2faaa26642a5e3a56e1ea103a0e19ad5fc1b2
SOURCE_CANDIDATE_PATH=V011/submission.asc
SOURCE_CANDIDATE_SHA256=cb1a8b0d0742a3427a0f5ccf7aa4619be5a2d60af5f82321bbe86fb34785c388
SOURCE_PARENT_BYTES=190077
SOURCE_CANDIDATE_BYTES=190108
SOURCE_DIFF=V011/diff-from-v008.patch; one changed declaration line

## Compile

BUILD_INPUT=existing V016 `CMakeLists.txt`, `device.cpp`, `adapter.cpp`,
`link_one.sh`, and the existing `runtime_probe.cpp` required by the
unchanged CMake probe target declarations. No CMake or Runner was created.
The remote temporary root was
`/tmp/cann-n02-code-footprint-icache-x-v011-always-inline`.

CMAKE_SHA256=7a5ee03a61a82eea0d76165a9aed4b076e50e19524bed8039b0e7bda6a5ee276
DEVICE_CPP_SHA256=8a1cda3888a18eb9fa82bde6c783f7710b5942937a16120ac04bc95f8201ef47
ADAPTER_CPP_SHA256=3a44ff3d4f5d9359e919421a7bfc01732727fdb3cd957572cc1dfe5a84bb0807
LINK_ONE_SHA256=582f8ed3a13a362cf3389a1f5df1f6a58461135efde74a26e8dd18e2cd6ecbf5
RUNTIME_PROBE_SHA256=e57518d3027097bc398f367486efdb46187de37d5d59fd26b43ed748feb53570

The first configure attempt returned 1 because the copied existing V016
CMake input declares `probe_v016` and `probe_v011` but the existing
`runtime_probe.cpp` had not yet been copied into the new temporary root. Its
complete stderr is retained in `logs/v011-compile.log`. The existing probe
source was then copied without modification, and the same CMake input and
targets were retried.

COMPILE=PASS_AFTER_EXISTING_INPUT_FIX
PARENT_CONFIG_RC=0
PARENT_BUILD_RC=0
CANDIDATE_CONFIG_RC=0
CANDIDATE_BUILD_RC=0
TARGETS=`device submission full_link`
HOST=hwnput3
TOOLCHAIN=clang 15.0.5 / bisheng
ENV_INIT=source /usr/local/Ascend/ascend-toolkit/set_env.sh with nounset disabled
ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest
SOC_TARGET=Ascend910B3

## Codegen Identity

CODEGEN_IDENTITY=DIFFERENT

| Artifact | Parent SHA256 / bytes | Candidate SHA256 / bytes | cmp |
| --- | --- | --- | ---: |
| `device.cpp.o` | `3e3266d0d3abffa5d0a48660f97f9a7c3abf9e8abb485d87617fe97c89c0bb57` / 334040 | `3f71519dd16f5c49eeefec1fb5663a60962e08156223b2c98835aa9eb10aea60` / 333696 | 1 |
| `adapter.cpp.o` | `7df79bf620d54da293e2f689dd2b913d7d46c5035681faf43d66ca6417d09064` / 334040 | `696b0190b1d811307a704c89e7c3632396ef3e2c69465050df6e3df45f79df74` / 333696 | 1 |
| `device.alink` | `998896a58bf828357e6885fb5eab3e41b79fcf70e898baeb3abf126f90be1b9f` / 183144 | `aa16afd376c387ddc36e130759c147bf885f50706f78b44b96ecdc2e6fbb79a9` / 183008 | 1 |
| `submission.alink` | `1e13fdfd6df7876db3820b2c299271d82aad5259f0e293ad540dd006a00a3630` / 183144 | `dd32839453c85618a94e47cf10c1136caf24608a4570bdc50a35ea4b043ab869` / 183016 | 1 |

The target object section
`.text._ZN20AddRmsNormBiasKernelIDhE33ProcessFp16FullRowOutputPipelinedEmmff_mix_aiv`
is 1884 bytes in Parent with SHA256
`34325787f19200975d60b9cef818192f9d98ccd38e3d37cad2f3a621882df8f3`.
The Candidate has no such object section and `llvm-objcopy` returned 1 for
that section, consistent with the function being inlined into its caller.

Final device `.text` changed from Parent SHA256
`e1e893d950450fdde3e22fb7ac239a50bda465aba66200c877649776891cc85a` /
77340 bytes to Candidate SHA256
`e504bdc19729d335e8e9ac5073249efa6a62e944101e1c02ce414e180dbafa13` /
77580 bytes. Raw `.text` comparison returned 1.

Final device objdump changed from Parent SHA256
`49e1ccd7d42db8f68f70c5c163ad2aacea3f03be1fd131516334c447627eba51` /
529045 bytes to Candidate SHA256
`7144e59704f0142b5f86f253a15f076527764e061e65c69592b92e5829876a00` /
530509 bytes. The submission objdump also changed from Parent SHA256
`56be0d4848270ef14c06a88d26a974add092667e3d89b48b66dc7d9a8e6dde9` /
529049 bytes to Candidate SHA256
`d4bb36dadb79a7cc7296aa7581afb9f227d430d9b6eb0c9ad9bd18da4615739a` /
530513 bytes. The raw artifact files are under `V011/artifacts/`.

ARTIFACT_AUDIT=PASS_WITH_EXPECTED_DIFFERENCES
The audit command ended with status 1 because its aggregate flag records the
expected `cmp=1` results and the absent Candidate hotspot section. All
successful `.text` extraction and objdump commands returned 0; the missing
Candidate section is the observed inlining result.

## Runtime

CORRECTNESS=NOT_RUN
LOCAL=NONE
DEVICE=NONE

The only existing V011 runtime source available in the reused V016 input is
the existing `runtime_probe.cpp`; it calls `aclrtResetDevice(device)` at line
262. No no-reset Correctness entry was available, so no device command was
started. No process, HBM state or device configuration was changed.

## Evidence And Next Action

- `logs/v011-compile.log`: first configure failure and complete stderr.
- `logs/v011-compile-retry.log`: repaired existing-input Compile and full
  command output.
- `logs/v011-artifact-audit.log`: object, hotspot section, alink, `.text` and
  objdump comparisons.
- `diff-from-v008.patch`: exact one-line source change.
- `artifacts/`: copied raw `.text`, objdump and Parent hotspot section files.

RESULT=REAL_EXECUTABLE_CODEGEN_CHANGE
PERFORMANCE_CLAIM=NONE
PUSH=NO
NEXT_ACTION=retain V011 as a real N02 code-footprint Candidate; use only an
existing no-reset Correctness entry if one becomes available, then evaluate
Local separately. Do not replace this result with V009 link-only evidence or
repeat noinline/function-order variants.
