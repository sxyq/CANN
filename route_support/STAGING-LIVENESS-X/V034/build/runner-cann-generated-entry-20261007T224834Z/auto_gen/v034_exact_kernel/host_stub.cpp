#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dlfcn.h>
#include <securec.h>

#ifndef ASCENDC_DUMP
#define ASCENDC_DUMP 1
#endif

#if defined(ASCENDC_DUMP) && (ASCENDC_DUMP == 0)
    #undef ASCENDC_DUMP
#endif

static char ascendcErrMsg[1024] = {0};

static void *g_kernel_handle_aiv = nullptr;

struct ascend_kernels {
    uint32_t version;
    uint32_t type_cnt;
    uint32_t aiv_type;
    uint32_t aiv_len;
    uint32_t aiv_file_len;
    uint8_t aiv_buf[__replaced_aiv_len];
} __replaced_ascend_kernel __attribute__ ((section ("__replaced_ascend_section"))) = {1,1,1,__replaced_aiv_len,__replaced_aiv_file_len,{0}};

extern "C" {
uint32_t RegisterAscendBinary(const char *fileBuf, size_t fileSize, uint32_t type, void **handle);
uint32_t LaunchAscendKernel(void *handle, const uint64_t key, const uint32_t blockDim, void **args,
                            uint32_t size, const void *stream);
uint32_t GetAscendCoreSyncAddr(void **addr);
int UnregisterAscendBinary(void *hdl);
void StartAscendProf(const char *name, uint64_t *startTime);
void ReportAscendProf(const char *name, uint32_t blockDim, uint32_t taskType, const uint64_t startTime);
bool GetAscendProfStatus();
uint32_t AllocAscendMemDevice(void **devMem, uint64_t size);
uint32_t FreeAscendMemDevice(void *devMem);
bool AscendCheckSoCVersion(const char *socVersion, char* errMsg);
void AscendProfRegister();
uint32_t GetCoreNumForMixVectorCore(uint32_t *aiCoreNum, uint32_t *vectorCoreNum);
uint32_t LaunchAscendKernelForVectorCore(const char *opType, void *handle, const uint64_t key, void **args, uint32_t size,
    const void *stream, bool enbaleProf, uint32_t aicBlockDim, uint32_t aivBlockDim, uint32_t aivBlockDimOffset);
}

namespace Adx {
    void AdumpPrintWorkSpace(const void *workSpaceAddr, const size_t dumpWorkSpaceSize,
                            void *stream, const char *opType);
}

    class KernelHandleGradUnregister {
    private:
        KernelHandleGradUnregister() {}

    public:
        KernelHandleGradUnregister(const KernelHandleGradUnregister&) = delete;
        KernelHandleGradUnregister& operator=(const KernelHandleGradUnregister&) = delete;

        static KernelHandleGradUnregister& GetInstance() {
            static KernelHandleGradUnregister instance;
            return instance;
        }
        ~KernelHandleGradUnregister(){
            if (g_kernel_handle_aiv) {
                UnregisterAscendBinary(g_kernel_handle_aiv);
                g_kernel_handle_aiv = nullptr;
            }
        }
    };

static void __register_kernels(void) __attribute__((constructor));
void __register_kernels(void)
{
    const char* compileSocVersion = "__replaced_ascend_compile_soc_version";
    uint32_t ret;

    bool checkSocVersion = AscendCheckSoCVersion(compileSocVersion, ascendcErrMsg);
    if (!checkSocVersion) {
        return;
    }
    ret = RegisterAscendBinary(
        (const char *)__replaced_ascend_kernel.aiv_buf,
        __replaced_ascend_kernel.aiv_file_len,
        1,
        &g_kernel_handle_aiv);
    if (ret != 0) {
        printf("RegisterAscendBinary aiv ret %u \n", ret);
    }

    AscendProfRegister();
}





uint32_t launch_and_profiling_add_rms_norm_bias_custom(uint64_t func_key, uint32_t blockDim, void* stream, void **args, uint32_t size)
{
    uint64_t startTime;
    const char *name = "add_rms_norm_bias_custom";
    bool profStatus = GetAscendProfStatus();
    if (profStatus) {
        StartAscendProf(name, &startTime);
    }
    if (g_kernel_handle_aiv == nullptr) {
        printf("[ERROR] %s\n", ascendcErrMsg);
        return 0;
    }
    uint32_t ret = LaunchAscendKernel(g_kernel_handle_aiv, func_key, blockDim, args, size, stream);
    if (ret != 0) {
        printf("LaunchAscendKernel ret %u\n", ret);
    }
    if (profStatus) {
        ReportAscendProf(name, blockDim, 5, startTime);
    }
    return ret;
}

template<typename T>
uint32_t aclrtlaunch_add_rms_norm_bias_custom(uint32_t blockDim, void* stream, void* x, void* residual, void* gamma, void* bias, void* output, uint64_t rowCount, uint64_t rowWidth, uint32_t blockCount, float invRowWidth, float epsilon);

template<>
uint32_t aclrtlaunch_add_rms_norm_bias_custom<float>(uint32_t blockDim, void* stream, void* x, void* residual, void* gamma, void* bias, void* output, unsigned long rowCount, unsigned long rowWidth, unsigned int blockCount, float invRowWidth, float epsilon)
{
    struct {
        alignas(((alignof(void*) + 3) >> 2) << 2) void* x;
        alignas(((alignof(void*) + 3) >> 2) << 2) void* residual;
        alignas(((alignof(void*) + 3) >> 2) << 2) void* gamma;
        alignas(((alignof(void*) + 3) >> 2) << 2) void* bias;
        alignas(((alignof(void*) + 3) >> 2) << 2) void* output;
        alignas(((alignof(unsigned long) + 3) >> 2) << 2) unsigned long rowCount;
        alignas(((alignof(unsigned long) + 3) >> 2) << 2) unsigned long rowWidth;
        alignas(((alignof(unsigned int) + 3) >> 2) << 2) unsigned int blockCount;
        alignas(((alignof(float) + 3) >> 2) << 2) float invRowWidth;
        alignas(((alignof(float) + 3) >> 2) << 2) float epsilon;
        alignas(((alignof(void*) + 3) >> 2) << 2) void* __ascendc_overflow;
    } __ascendc_args;

    uint32_t __ascendc_ret;
    constexpr uint32_t __ascendc_overflow_status_size = 8;
    AllocAscendMemDevice(&(__ascendc_args.__ascendc_overflow), __ascendc_overflow_status_size);
    __ascendc_args.x = x;
    __ascendc_args.residual = residual;
    __ascendc_args.gamma = gamma;
    __ascendc_args.bias = bias;
    __ascendc_args.output = output;
    __ascendc_args.rowCount = rowCount;
    __ascendc_args.rowWidth = rowWidth;
    __ascendc_args.blockCount = blockCount;
    __ascendc_args.invRowWidth = invRowWidth;
    __ascendc_args.epsilon = epsilon;

    __ascendc_ret = launch_and_profiling_add_rms_norm_bias_custom(0, blockDim, stream, (void **)&__ascendc_args, sizeof(__ascendc_args));
    KernelHandleGradUnregister::GetInstance();
    FreeAscendMemDevice(__ascendc_args.__ascendc_overflow);
    return __ascendc_ret;
}

template<>
uint32_t aclrtlaunch_add_rms_norm_bias_custom<half>(uint32_t blockDim, void* stream, void* x, void* residual, void* gamma, void* bias, void* output, unsigned long rowCount, unsigned long rowWidth, unsigned int blockCount, float invRowWidth, float epsilon)
{
    struct {
        alignas(((alignof(void*) + 3) >> 2) << 2) void* x;
        alignas(((alignof(void*) + 3) >> 2) << 2) void* residual;
        alignas(((alignof(void*) + 3) >> 2) << 2) void* gamma;
        alignas(((alignof(void*) + 3) >> 2) << 2) void* bias;
        alignas(((alignof(void*) + 3) >> 2) << 2) void* output;
        alignas(((alignof(unsigned long) + 3) >> 2) << 2) unsigned long rowCount;
        alignas(((alignof(unsigned long) + 3) >> 2) << 2) unsigned long rowWidth;
        alignas(((alignof(unsigned int) + 3) >> 2) << 2) unsigned int blockCount;
        alignas(((alignof(float) + 3) >> 2) << 2) float invRowWidth;
        alignas(((alignof(float) + 3) >> 2) << 2) float epsilon;
        alignas(((alignof(void*) + 3) >> 2) << 2) void* __ascendc_overflow;
    } __ascendc_args;

    uint32_t __ascendc_ret;
    constexpr uint32_t __ascendc_overflow_status_size = 8;
    AllocAscendMemDevice(&(__ascendc_args.__ascendc_overflow), __ascendc_overflow_status_size);
    __ascendc_args.x = x;
    __ascendc_args.residual = residual;
    __ascendc_args.gamma = gamma;
    __ascendc_args.bias = bias;
    __ascendc_args.output = output;
    __ascendc_args.rowCount = rowCount;
    __ascendc_args.rowWidth = rowWidth;
    __ascendc_args.blockCount = blockCount;
    __ascendc_args.invRowWidth = invRowWidth;
    __ascendc_args.epsilon = epsilon;

    __ascendc_ret = launch_and_profiling_add_rms_norm_bias_custom(1000000, blockDim, stream, (void **)&__ascendc_args, sizeof(__ascendc_args));
    KernelHandleGradUnregister::GetInstance();
    FreeAscendMemDevice(__ascendc_args.__ascendc_overflow);
    return __ascendc_ret;
}

template<>
uint32_t aclrtlaunch_add_rms_norm_bias_custom<__bf16>(uint32_t blockDim, void* stream, void* x, void* residual, void* gamma, void* bias, void* output, unsigned long rowCount, unsigned long rowWidth, unsigned int blockCount, float invRowWidth, float epsilon)
{
    struct {
        alignas(((alignof(void*) + 3) >> 2) << 2) void* x;
        alignas(((alignof(void*) + 3) >> 2) << 2) void* residual;
        alignas(((alignof(void*) + 3) >> 2) << 2) void* gamma;
        alignas(((alignof(void*) + 3) >> 2) << 2) void* bias;
        alignas(((alignof(void*) + 3) >> 2) << 2) void* output;
        alignas(((alignof(unsigned long) + 3) >> 2) << 2) unsigned long rowCount;
        alignas(((alignof(unsigned long) + 3) >> 2) << 2) unsigned long rowWidth;
        alignas(((alignof(unsigned int) + 3) >> 2) << 2) unsigned int blockCount;
        alignas(((alignof(float) + 3) >> 2) << 2) float invRowWidth;
        alignas(((alignof(float) + 3) >> 2) << 2) float epsilon;
        alignas(((alignof(void*) + 3) >> 2) << 2) void* __ascendc_overflow;
    } __ascendc_args;

    uint32_t __ascendc_ret;
    constexpr uint32_t __ascendc_overflow_status_size = 8;
    AllocAscendMemDevice(&(__ascendc_args.__ascendc_overflow), __ascendc_overflow_status_size);
    __ascendc_args.x = x;
    __ascendc_args.residual = residual;
    __ascendc_args.gamma = gamma;
    __ascendc_args.bias = bias;
    __ascendc_args.output = output;
    __ascendc_args.rowCount = rowCount;
    __ascendc_args.rowWidth = rowWidth;
    __ascendc_args.blockCount = blockCount;
    __ascendc_args.invRowWidth = invRowWidth;
    __ascendc_args.epsilon = epsilon;

    __ascendc_ret = launch_and_profiling_add_rms_norm_bias_custom(2000000, blockDim, stream, (void **)&__ascendc_args, sizeof(__ascendc_args));
    KernelHandleGradUnregister::GetInstance();
    FreeAscendMemDevice(__ascendc_args.__ascendc_overflow);
    return __ascendc_ret;
}
