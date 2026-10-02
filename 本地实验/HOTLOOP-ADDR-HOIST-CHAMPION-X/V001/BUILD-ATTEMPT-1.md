# V001 Build Attempt 1

```text
REVISION=V001
SOURCE_COMMIT=d1de6208
SOURCE_SHA256=26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602
HOST=hwnput3
DATE_UTC=2026-10-02
BUILD_KIND=LOCAL_HWNPUT3_EXACT_SOURCE_CHAIN
SERVER3_ALIAS=cann-server3
SERVER3_ACCESS=BLOCKED_DNS_TEMPORARY_FAILURE
```

## Result

`CMAKE_CONFIGURE=PASS`; `ASC_COMPILE=FAIL`; `LINK=NOT_REACHED`.

The exact compiler error was:

```text
/tmp/asc_plugin_binary_register_code-901334.c:3:10: fatal error: 'vector' file not found
```

The subsequent `cceld` and `ld.lld` errors are compiler-failure cascades.

## Commands and evidence

- Preflight: `logs/build-preflight-local-hwnput3-20261002.txt`
- Configure: `logs/build-configure-local-hwnput3-20261002.txt`
- Compile/link: `logs/build-compile-local-hwnput3-20261002.txt`
- Build directory retained as local diagnostic artifact: `build-v001/`

The preflight recorded `FREE_HBM` values from the supplied/current local
snapshot and the live `npu-smi`/process state. No process was stopped.

## Disposition

`BUILD_FAILED` is an environment/build-chain failure at the ASC compiler
front end, not a correctness or performance verdict. The retry build fixes
only compiler identity by pinning CMake to
`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng`.
No H3 kernel mechanism is changed by this fix.
