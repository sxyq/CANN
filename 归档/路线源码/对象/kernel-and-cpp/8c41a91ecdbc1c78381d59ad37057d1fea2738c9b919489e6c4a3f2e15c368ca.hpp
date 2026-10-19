#ifndef H001_COMPILE_ADAPTER_HPP
#define H001_COMPILE_ADAPTER_HPP

// Local-only stand-ins for types/APIs the judge predefines before kernel.asc.
// Field layout matches npu_kernel_dev / judge.asc. Never submitted.

#include <cstdint>
#include <cstddef>

struct TensorInfo {
    const int64_t *shape;
    int64_t numDims;
    int32_t dtype;
};

struct TensorGroupInfo {
    const TensorInfo *tensors;
    int64_t numTensors;
};

#ifndef ACLRT_STREAM_DEFINED_BY_ADAPTER
#define ACLRT_STREAM_DEFINED_BY_ADAPTER
typedef void *aclrtStream;
#endif

#ifndef ACL_SUCCESS
#define ACL_SUCCESS 0
#endif
#ifndef ACL_MEM_MALLOC_HUGE_FIRST
#define ACL_MEM_MALLOC_HUGE_FIRST 1
#endif
#ifndef ACL_MEMCPY_HOST_TO_DEVICE
#define ACL_MEMCPY_HOST_TO_DEVICE 1
#endif

inline int aclrtMalloc(void **ptr, size_t size, int flag)
{
    (void)flag;
    *ptr = nullptr;
    return size ? 0 : 1;
}
inline int aclrtMemcpy(void *dst, size_t dstMax, const void *src, size_t count, int kind)
{
    (void)dst;
    (void)dstMax;
    (void)src;
    (void)count;
    (void)kind;
    return 0;
}
inline int aclrtFree(void *ptr)
{
    (void)ptr;
    return 0;
}
inline int aclrtSynchronizeStream(aclrtStream stream)
{
    (void)stream;
    return 0;
}

#endif
