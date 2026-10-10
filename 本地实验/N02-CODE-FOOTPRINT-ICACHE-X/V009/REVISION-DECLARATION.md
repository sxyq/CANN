# N02 V009 Revision Declaration

ROUTE: N02 CODE-FOOTPRINT-ICACHE-X
REVISION: V009
PARENT: V008 / ca28605bcf1fed9a9c43706f881d6c322f902c75
WORKTREE: /Users/sunyiyang/Desktop/Project/cann/worktrees/n02-code-footprint-icache
BRANCH: route/n02-code-footprint-icache-x

## Change

This revision is a toolchain/linker capability experiment. It makes no device
source change and keeps the V016 Parent/Candidate source inputs identical.
The single link factor is the existing `ld.lld 15.0.5` option
`--symbol-ordering-file`, applied only to the Candidate direct link. The
existing `link_one.sh` remains the default Parent/Candidate link entry.

The order file contains the two existing FP16 hot-path symbols:

```text
_ZN20AddRmsNormBiasKernelIDhE33ProcessFp16FullRowOutputPipelinedEmmff_mix_aiv
_ZN20AddRmsNormBiasKernelIDhE23ProcessWideLowPrecisionEmmjff_mix_aiv
```

Source input SHA256: `4a2788e57efe4a0019a68928fec2faaa26642a5e3a56e1ea103a0e19ad5fc1b2`
for both Parent and Candidate. `device.cpp`, `adapter.cpp`, shape paths,
arithmetic, DMA and synchronization are unchanged from V008.

## Compile and Link

COMPILE: PASS
LINK: PASS
CONFIGURE: Parent 0, Candidate 0
BUILD: Parent 0, Candidate 0
TARGETS: `device submission full_link`
HOST: `hwnput3` / server3
TOOLCHAIN: CANN 8.5.0.alpha002, `clang 15.0.5`, `ld.lld 15.0.5`
ENV: `source /usr/local/Ascend/ascend-toolkit/set_env.sh` returned 0;
`ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest`;
`--npu-soc=Ascend910B3` was present in the compile commands.

The fresh object comparison is equal:

| Object | Parent/Candidate SHA256 | Size | cmp |
| --- | --- | ---: | ---: |
| `device.cpp.o` | `3e3266d0d3abffa5d0a48660f97f9a7c3abf9e8abb485d87617fe97c89c0bb57` | 334040 | 0 |
| `adapter.cpp.o` | `7df79bf620d54da293e2f689dd2b913d7d46c5035681faf43d66ca6417d09064` | 334040 | 0 |

The default alinks are equal, while the Candidate ordered alinks differ from
the Parent defaults:

| Artifact | SHA256 | Size | Comparison |
| --- | --- | ---: | --- |
| Parent `device.default.alink` | `998896a58bf828357e6885fb5eab3e41b79fcf70e898baeb3abf126f90be1b9f` | 183144 | default Candidate equal |
| Candidate `device.symbol-order.alink` | `def3ad8b94ca17e0c3d6dee68f952d4dfd2e023df1668a4348a5ff0b9cbdffa2` | 183144 | ordered differs, cmp 1 |
| Parent `submission.default.alink` | `1e13fdfd6df7876db3820b2c299271d82aad5259f0e293ad540dd006a00a3630` | 183144 | default Candidate equal |
| Candidate `submission.symbol-order.alink` | `b74de715cd17e3ebc3b7891e65840b6f4f2c9c55059d21cda8f2336a31f37696` | 183144 | ordered differs, cmp 1 |

The executable `.text` section, which contains the target FP16 hot-path
symbols, remains 77340 bytes but has different raw bytes:

| Artifact | SHA256 | Size |
| --- | --- | ---: |
| Parent device `.text` | `e1e893d950450fdde3e22fb7ac239a50bda465aba66200c877649776891cc85a` | 77340 |
| Candidate ordered device `.text` | `92183b9370873319a4e961dbf04f2440e15651f0d8522d3120450cef595c3379` | 77340 |

Raw `.text` comparison returned 1. `llvm-objdump --disassemble --section=.text`
also differs: Parent output SHA256
`5bb0a87af10baee0529a839b8bfb00ef14ed399689996509c261c57705c2dbb4`,
Candidate output SHA256
`1378e59ea1b7ed28a96257cccae88d31c27b9c47db6665c48ef657c788fddabc`.
The device objdump comparison returned 1. The same result was observed for
the submission artifact. The linker output is therefore an actual layout
change, not an option-only probe.

## Runtime Result

CORRECTNESS: NOT_RUN
LOCAL: NONE
DEVICE: NONE
NPU: no device command was run; no process, HBM or AICore state was changed.
PERFORMANCE: no performance claim is made.

## Evidence

- `logs/v009-fresh-compile.log`: fresh V016 configure/build output.
- `logs/v009-linker-capability-probe.log`: `ld.lld` option support and symbol inventory.
- `logs/v009-fresh-symbol-order-link.log`: fresh environment, commands, stderr,
  return codes and alink SHA256/size.
- `logs/v009-fresh-artifact-audit.log`: object/alink/text/objdump comparisons
  and section sizes.
- `logs/v009-hotspot-tool-probe.log`: existing `llvm-objdump --syms` tool
  result and the unavailable `llvm-nm` path.
- `logs/v009-symbol-ordering.txt`: exact linker order file.
- `logs/v009-symbol-order-link.log`: retained preliminary log from the stale
  V008 temporary directory; it is not used for this declaration.

NEXT_ACTION: stop new N02 Candidate revisions; Main may review this linker
layout capability result. No push.
