# N02 V010 V008 Feasibility Review

ROUTE=N02 CODE-FOOTPRINT-ICACHE-X
REVISION=V010-AUDIT-ONLY
PARENT_REVISION=V009
PARENT_COMMIT=3035fa9afe922950725aecd6fe8616e62ba07df5
RULE_REFRESH_RECEIPT=ROUTE=N02 CODE-FOOTPRINT-ICACHE-X; HEAD=3035fa9afe922950725aecd6fe8616e62ba07df5; PARENT=ca28605bcf1fed9a9c43706f881d6c322f902c75; MECHANISM=executable hotspot footprint and instruction-fetch layout; FORBIDDEN=R029 runtime dispatch, repeated noinline, equivalent source reordering, section metadata only, unsupported basic-block option, new Runner, new ledger, new scheduler; NEXT_ACTION=review V008 runtime feasibility and retain exact source/codegen identity

## Scope

This record reviews the existing V008 source Candidate and its existing V016
build evidence. It creates no source Candidate and does not restart from V001.
V009 remains link-only evidence for the supported `ld.lld
--symbol-ordering-file` layout option; it is not a source Candidate or a
runtime result.

## Source And Build Input

Parent source:

- Local path: `V004/submission.asc`
- Remote V008 input: `/tmp/cann-n02-code-footprint-icache-x-v008-order/parent/R31B-V016-WIDE-TILE-SEED_kernel.asc`
- SOURCE_SHA256=`75aef181ceb2822c69fb32107b6cc20ac4dcc88d13a1fd727bb7755785314f2d`

Candidate source:

- Local path: `V008/submission.asc`
- Remote V008 input: `/tmp/cann-n02-code-footprint-icache-x-v008-order/candidate/R31B-V016-WIDE-TILE-SEED_kernel.asc`
- SOURCE_SHA256=`4a2788e57efe4a0019a68928fecf2aaa26642a5e3a56e1ea103a0e19ad5fc1b2`

Both source files are 190077 bytes. V008 changes only the definition order of
the six existing synchronization helpers; their bodies, call sites, shape
predicates, arithmetic, DMA operations and synchronization semantics remain
unchanged. The existing V016 inputs were `CMakeLists.txt`, `device.cpp`,
`adapter.cpp` and `link_one.sh`, with targets `device submission full_link`.

## Results

COMPILE=PASS_INHERITED_FROM_V008
PARENT_CONFIG_RC=0
PARENT_BUILD_RC=0
CANDIDATE_CONFIG_RC=0
CANDIDATE_BUILD_RC=0
CORRECTNESS=NOT_RUN
LOCAL=NONE

CODEGEN_IDENTITY=V008_PARENT_CANDIDATE_EQUAL

- `device.cpp.o`: Parent/Candidate SHA256
  `3e3266d0d3abffa5d0a48660f97f9a7c3abf9e8abb485d87617fe97c89c0bb57`,
  334040 bytes, `cmp=0`.
- `adapter.cpp.o`: Parent/Candidate SHA256
  `7df79bf620d54da293e2f689dd2b913d7d46c5035681faf43d66ca6417d09064`,
  334040 bytes, `cmp=0`.
- `device.alink`: Parent/Candidate SHA256
  `998896a58bf828357e6885fb5eab3e41b79fcf70e898baeb3abf126f90be1b9f`,
  183144 bytes, `cmp=0`.
- `submission.alink`: Parent/Candidate SHA256
  `1e13fdfd6df7876db3820b2c299271d82aad5259f0e293ad540dd006a00a3630`,
  183144 bytes, `cmp=0`.
- FP16 `ProcessWideLowPrecision` section: 4660 bytes, both SHA256
  `662a9e8647eb1960c4e007988b32ece619f2160204f268b287a7e519b65a2d50`,
  raw `cmp=0`.
- Hotspot objdump: both SHA256
  `3f90d601333322b6d14e0a06a1ce2b430b91dc3d39be02a934f9984620467474`,
  30559 bytes, `cmp=0`.
- Final `.text`: both 77340 bytes with SHA256
  `e1e893d950450fdde3e22fb7ac239a50bda465aba66200c877649776891cc85a`,
  raw `cmp=0`.

## Runtime Feasibility

The existing V008 remote runtime entry is
`/tmp/cann-n02-code-footprint-icache-x-v008-order/{parent,candidate}/runtime_probe.cpp`.
Both copies have SHA256
`e57518d3027097bc398f367486efdb46187de37d5d59fd26b43ed748feb53570`.
The existing entry calls `aclrtResetDevice(device)` at line 262. The V008
temporary directory contains these two `runtime_probe.cpp` files and no
separate paired Runner.

The read-only server3 snapshot at `2026-10-10T20:54:04+00:00` reported NPU3
910B3 with AICore 51%, HBM 63351/65536 MB used, and Python PID 658670 using
59977 MB. The schedule table had no current N02 lease for device 3, but the
live process and HBM state make a reset-capable runtime entry unsuitable. No
Correctness or Local command was started, and no process was changed.

The V008 temporary build directory has since been reused. Its current Parent
device object is `a9e65df9f23780ea7104532abe78079aef08990e49049040ea476014f08496dd`
(319920 bytes) and current Parent device alink is
`6a56f50d8e78de2dcae64f6d078a8c2f898f0947a004c568aa922b445cae9a0f`
(186816 bytes). These differ from the committed V008 identities above and
are excluded from V008 conclusions.

## Evidence And Next Action

Evidence:

- `V008/REVISION-DECLARATION.md` and `V008/logs/` retain the formal Compile
  and codegen comparison.
- `V009/REVISION-DECLARATION.md` and `V009/logs/` retain the link-only
  `--symbol-ordering-file` result.
- `V010/logs/v010-preflight-server3.log` retains the environment and NPU
  snapshot.
- `V010/logs/v010-source-codegen-audit.log` retains the remote input and
  temporary-output identity audit.
- `V010/logs/v010-runner-safety-audit.log` retains the existing runtime entry,
  reset call and temporary-directory inventory.

RESULT=V008_COMPILE_PASS_CODEGEN_EQUAL_RUNTIME_NOT_RUN
PERFORMANCE_CLAIM=NONE
PUSH=NO
NEXT_ACTION=continue N02 code-footprint and instruction-fetch research with
a new source factor only after it can produce a real executable codegen
difference through the existing V016 build input. Do not repeat V008 function
order or V001/V003/V004 noinline variants; do not use V009 link-only output as
a source Candidate.
