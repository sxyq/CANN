# Build, `.ascend.meta`, And DFX Notes

Use this file only when the task mentions compile-time metadata, `.ascend.meta`, TLV, DFX, or feature markers for debug instrumentation.

## Repository Compiler Entry Points

- `${DEVKIT_PATH}/tools/build/asc_op_compile_base/asc_op_compiler/kernel_info_infer.py`
- `${DEVKIT_PATH}/tools/build/asc_op_compile_base/asc_op_compiler/compile_op.py`
- `${DEVKIT_PATH}/tools/build/asc_op_compile_base/asc_op_compiler/ascendc_compile_base.py`
- `${DEVKIT_PATH}/tools/build/asc_op_compile_base/asc_op_compiler/ascendc_compile_dfx.py`

## What The Repository Sources Indicate

- `kernel_info_infer.py` infers kernel-side debug features such as compile markers for `printf` and `assert`.
- `compile_op.py` stores `ascendc_meta_info` and participates in appending metadata sections.
- `ascendc_compile_base.py` contains the relocatable meta-file link step.
- `ascendc_compile_dfx.py` is the most relevant repository source when the task involves TLV-like debug metadata generation.

## Observed DFX Constants

The repository `ascendc_compile_dfx.py` defines these constants:

```text
F_TYPE_KTYPE = 1
F_TYPE_L0_EXCEPTION_DFX = 4
F_TYPE_L0_EXCEPTION_DFX_ARGSINFO = 5
F_TYPE_L0_EXCEPTION_DFX_IS_TIK = 6
```

Treat these as repository ground truth when discussing existing DFX metadata behavior for the task.

## Practical Rule

- Do not invent a new `.ascend.meta` layout from memory.
- If the task needs metadata changes, derive the change from the repository compiler code first.
- If the task only needs normal AscendC kernel development, do not load this file.
