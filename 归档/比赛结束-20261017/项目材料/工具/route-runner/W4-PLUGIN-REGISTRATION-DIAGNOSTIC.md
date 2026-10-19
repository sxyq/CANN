# W4 Plugin Registration Diagnostic

Branch: `tooling/canonical-route-runner`

Scope: tooling-owned runner only. No route worktree, score, or dashboard was accessed.

## Attempt

Built the tooling-owned FP32 fixture as both exact-source inputs with CANN `8.5.T8.0.B060`, `dav-2201`, and SHA-256 `edeb3b01466431a2e6ccfd19f564a7fe303642cd1ba3c23a3a1580e5c7179c94`. The build used an isolated `/tmp` directory. The generated `bisheng` rule includes the HCC C++ 7.3 include roots both before the architecture flags and inside `-Xhost-start`.

The generated registration translation unit `/tmp/asc_plugin_binary_register_code-6a5bc8.c` was separately syntax-checked with those HCC roots and the SDK include roots. `<vector>` resolved; the probe then stopped on undefined `fatbinDataPtr` and `fatbinDataLength`, which are expected from the incomplete device-binary generation below.

## Result

The ASC build failed before producing shared libraries or an executable:

```text
[ERROR] ASCPLUGIN ... Unknown kernelInfo, mangledName: add_custom_kernel ...
ld.lld: error: undefined symbol: __origin__add_custom_kernel
```

Thus the earlier `vector: file not found` diagnostic was not reproduced with the HCC paths on the actual `bisheng` rule, but this attempt does not establish a successful plugin-registration build. The current blocker is kernel-info resolution and the resulting absent device binary. Executable smoke, kernel correctness, and device-event paired Local were not run; no score was produced.
