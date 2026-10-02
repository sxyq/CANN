# ADDR H3 V001 Known-Good Correctness Cross-Check

```text
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
REVISION=V001
PURPOSE=SUPPORT_ONLY_CORRECTNESS_DIAGNOSTIC
CANDIDATE_KERNEL_EDIT=NONE
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602
```

## Proven historical runner

The runner body is reused from:

```text
ORIGIN_CPP=本地实验/R31B/V016/support/paired/paired_runner.cpp
ORIGIN_CPP_SHA256=670ba116f04348ba442aa5717a9afe5d724a358c777483404629838ba76b65f3
ORIGIN_ABI=本地实验/R31B/V016/support/paired/paired_runner_abi.h
ORIGIN_ABI_SHA256=096c0229f5d9e6fa37460808387517280cb0f2e4296e07b8c37f75cd73c89617
```

That runner historically bound the exact R31B V011 Parent, used the existing
BF16 conversion and CPU `CheckOutput` reference, and recorded device-event
measurements. Its retained correctness evidence passed the Parent/Candidate
matrix through BF16-D32768.

## Support-only adaptation

Only the following support bindings were changed:

1. The wrapper binds R31B V011 as `r31b_v011::run_kernel_v011` and the
   unchanged ADDR H3 V001 source as `r31b_v016::run_kernel_v016`.
2. The proven runner row count changes from 2 to 9 to match the ADDR V001
   D40960 correctness geometry (eight planned rows plus one tail row).
3. The case list is reduced to one BF16 width-40960 case.

The reference arithmetic, dtype encoders/decoders, `CheckOutput`, ACL launch
sequence, source-bound Parent wrapper, and runtime protocol remain the
historical runner implementation. This diagnostic is not a Formal Local
performance result; any event timing emitted by the reused runner is ignored
for scoring and used only to reach/check the two binaries.
