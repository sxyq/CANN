#include "paired_probe_bridge.h"
#include <cstdio>
#include <dlfcn.h>

namespace {
PairedHostEntry LoadHostEntry(const char* library, const char* symbol)
{
    dlerror();
    void* module = dlopen(library, RTLD_NOW | RTLD_LOCAL);
    if (module == nullptr) {
        const char* error = dlerror();
        fprintf(stderr, "dlopen(%s) failed: %s\n", library,
                error == nullptr ? "unknown loader error" : error);
        return nullptr;
    }
    dlerror();
    void* address = dlsym(module, symbol);
    const char* error = dlerror();
    if (error != nullptr || address == nullptr) {
        fprintf(stderr, "dlsym(%s) failed: %s\n", symbol,
                error == nullptr ? "symbol resolved to null" : error);
        dlclose(module);
        return nullptr;
    }
    return reinterpret_cast<PairedHostEntry>(address);
}
}  // namespace

extern "C" bool r31a_run_kernel_v016(
    void* x, const TensorGroupInfo& info_x,
    void* residual, const TensorGroupInfo& info_residual,
    void* gamma, const TensorGroupInfo& info_gamma,
    void* bias, const TensorGroupInfo& info_bias,
    void* output, const TensorGroupInfo& info_output,
    int64_t availableCoreNum, aclrtStream stream, float epsilon)
{
    static PairedHostEntry entry = LoadHostEntry(
        "./libr31a_paired_v016.so", "r31a_host_entry_v016");
    if (entry == nullptr) return false;
    entry(x, info_x, residual, info_residual, gamma, info_gamma,
          bias, info_bias, output, info_output, availableCoreNum, stream, epsilon);
    return true;
}

extern "C" bool r31a_run_kernel_v021(
    void* x, const TensorGroupInfo& info_x,
    void* residual, const TensorGroupInfo& info_residual,
    void* gamma, const TensorGroupInfo& info_gamma,
    void* bias, const TensorGroupInfo& info_bias,
    void* output, const TensorGroupInfo& info_output,
    int64_t availableCoreNum, aclrtStream stream, float epsilon)
{
    static PairedHostEntry entry = LoadHostEntry(
        "./libr31a_paired_v021.so", "r31a_host_entry_v021");
    if (entry == nullptr) return false;
    entry(x, info_x, residual, info_residual, gamma, info_gamma,
          bias, info_bias, output, info_output, availableCoreNum, stream, epsilon);
    return true;
}

#ifndef R31A_PAIRED_PROBE_BRIDGE_ONLY
#include "paired_probe_main.inc"
#endif
