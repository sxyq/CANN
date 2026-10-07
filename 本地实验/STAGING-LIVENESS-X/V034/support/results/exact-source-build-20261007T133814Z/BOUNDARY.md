# V034 Exact-Source Runner Boundary

- Route/revision: STAGING-LIVENESS-X / V034
- Gate: route-local exact-source runner build/link
- Result: `BUILD_FAILED`; no executable was produced.
- Candidate SHA-256 before and after the attempt: `75131108d32bc587c3c289d53ded04a15c5224417c675cb37a46df032e104d33`.
- Parent V033 SHA-256: `0df982248c80c4f2e5541dc730b0a25ba97544efc3b3e3e3409c24deed2d2eff`.
- Environment: installed CANN `8.5.0.alpha002`, Bisheng/ASC compiler reported as Clang 15.0.5; environment loaded from the installed `aarch64-linux/script/set_env.sh`.

The host runner calls the source-defined `extern "C" run_kernel` ABI. The independent ASC adapter includes the SHA-pinned source verbatim and defines no replacement kernel or dtype launch wrapper. The build reaches ASC plugin lowering, then rejects the original templated global launch at `submission.asc:3468` with `Unknown kernelInfo` for `add_rms_norm_bias_custom`. Linking also reports missing `__origin__add_rms_norm_bias_custom<T>` definitions for `float`, `half`, and `bfloat16_t`.

This is the boundary for the current no-Candidate-edit approach: the installed ASC lowering path does not provide callable metadata/definitions for the source's templated launch entry. There is no exact-source executable to run. No V034 or V033 NPU correctness comparison was made, and no device smoke was started.

Disposition: V034 correctness remains unverified, not `CORRECTNESS_FAILED`; Local is not run and has no claim. Earlier class-reinvoking wrapper diagnostics are not exact-kernel evidence and were excluded. Do not continue speculative launch ABI changes or use a copied/transformed kernel. Further progress requires a separately validated compiler/runtime mechanism for this exact source ABI, or explicit authorization for a Candidate interface change.

Evidence:

- `configure-candidate.log`
- `build-candidate.log`
- `../exact-source-build-20261007T132847Z/build-candidate.log` (initial compile/link diagnostic)
- `../exact-source-build-20261007T133135Z/build-candidate.log` (ASC device-object/host-link architecture mismatch)
