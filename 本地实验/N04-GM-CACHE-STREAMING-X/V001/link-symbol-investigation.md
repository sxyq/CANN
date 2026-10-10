# N04 V001 Link Symbol Investigation

## Outcome

COMPILE=FAIL/LINK_FAIL. `device` and `submission` compile; `full_link` exits 2 because `g_opSystemRunCfg` is undefined. Candidate source is unchanged. Correctness was not run and Local remains NONE. No further link attempt was made.

## CANN 8.5.0.alpha002 evidence

- `aarch64-linux/ascendc/include/basic_api/interface/kernel_tensor.h:240` declares `GlobalTensor::SetL2CacheHint`.
- `aarch64-linux/ascendc/include/basic_api/impl/kernel_tensor_impl.h:1489-1497` routes the call to `L2CacheAlter`.
- `aarch64-linux/ascendc/include/basic_api/impl/utils/kernel_utils_macros.h:183` declares `extern __gm__ struct OpSystemRunCfg g_opSystemRunCfg` under `L2_CACHE_HINT`. For NPU architecture 2201, the `L2_CACHE_HINT` branch at lines 272-276 reads `g_opSystemRunCfg.l2Cacheoffset`.
- An installed CANN sample at `opp/built-in/op_impl/ai_core/tbe/impl/ascendc/3rd/template_linear_algebra/include/template_linear_algebra/gemm/kernel/grouped_matmul_slice_m.hpp:20` supplies the symbol in that kernel source as `g_opSystemRunCfg{Catlass::L2_OFFSET}`; the same file calls `SetL2CacheHint(CACHE_MODE_DISABLE)` at line 126. The duplicate Catlass sample path under `common/catlass` also contains a source definition.
- CANN's `ASC_CMake/FindASC.cmake:60-80` implements `ascendc_fatbin_library` by linking `$<TARGET_OBJECTS:${target_name}>`; `util/merge_obj.sh:69-70` likewise passes the selected object list to `ld.lld`. No generic configuration object is appended by these targets.
- The reused R31B V016 `CMakeLists.txt:61-69` invokes `link_one.sh` with only the `device` object for `device.alink`. `link_one.sh:9-10` invokes static `ld.lld` with only `"$@"`.
- `nm -u` on the N04 `device.cpp.o` reports `U g_opSystemRunCfg`. A defined-symbol scan of the CANN 8.5 `aarch64-linux/lib64`, `runtime/lib64`, `compiler/lib64`, and `tools` trees returned no provider.

## Conclusion

With `L2_CACHE_HINT` enabled for DAV-2201, `SetL2CacheHint` introduces a real reference to `g_opSystemRunCfg`. The existing CANN sample carries its own Catlass-specific definition in the kernel source; it is not a general runtime-library definition. The reused V016 entry and the installed ASC object-link target do not add a configuration object. No matching provider is present in the N04 build inputs, so the observed `FAIL/LINK_FAIL` is retained. No symbol was added to Candidate or supplied by a fabricated object.

## Search scope

Read-only queries on server3, host `hwnput3`, using the installed CANN 8.5.0.alpha002 headers, ASC CMake modules, shipped AscendC/OPP sources, existing R31B V016 build input, and the N04 object produced by the recorded build. The CANN library symbol scan was limited to the four library/tool trees listed above. No source, object, CMake input, or server process was changed during this investigation.
