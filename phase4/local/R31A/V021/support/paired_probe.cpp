#include "probe_prelude.inc"
#include <cstdint>
#include <cstdio>
#include <dlfcn.h>

using PairedHostEntry = void (*)(
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, int64_t, aclrtStream, float);

namespace {
PairedHostEntry LoadHostEntry(const char* library, const char* symbol)
{
    void* module = dlopen(library, RTLD_NOW | RTLD_LOCAL);
    if (module == nullptr) {
        fprintf(stderr, "dlopen(%s) failed: %s\n", library, dlerror());
        return nullptr;
    }
    void* address = dlsym(module, symbol);
    if (address == nullptr) {
        fprintf(stderr, "dlsym(%s) failed: %s\n", symbol, dlerror());
        return nullptr;
    }
    return reinterpret_cast<PairedHostEntry>(address);
}
}  // namespace

extern "C" void r31a_run_kernel_v016(
    void* x, const TensorGroupInfo& info_x,
    void* residual, const TensorGroupInfo& info_residual,
    void* gamma, const TensorGroupInfo& info_gamma,
    void* bias, const TensorGroupInfo& info_bias,
    void* output, const TensorGroupInfo& info_output,
    int64_t availableCoreNum, aclrtStream stream, float epsilon)
{
    static PairedHostEntry entry = LoadHostEntry(
        "./libr31a_paired_v016.so", "r31a_host_entry_v016");
    if (entry != nullptr) {
        entry(x, info_x, residual, info_residual, gamma, info_gamma,
              bias, info_bias, output, info_output, availableCoreNum, stream, epsilon);
    }
}

extern "C" void r31a_run_kernel_v021(
    void* x, const TensorGroupInfo& info_x,
    void* residual, const TensorGroupInfo& info_residual,
    void* gamma, const TensorGroupInfo& info_gamma,
    void* bias, const TensorGroupInfo& info_bias,
    void* output, const TensorGroupInfo& info_output,
    int64_t availableCoreNum, aclrtStream stream, float epsilon)
{
    static PairedHostEntry entry = LoadHostEntry(
        "./libr31a_paired_v021.so", "r31a_host_entry_v021");
    if (entry != nullptr) {
        entry(x, info_x, residual, info_residual, gamma, info_gamma,
              bias, info_bias, output, info_output, availableCoreNum, stream, epsilon);
    }
}

#include "paired_probe_main.inc"
