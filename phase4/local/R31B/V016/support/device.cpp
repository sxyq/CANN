#include <cstdint>
#include <cstddef>

#ifndef TENSOR_GROUP_INFO_DEFINED
#define TENSOR_GROUP_INFO_DEFINED
struct TensorInfo { const int64_t* shape; int64_t numDims; int32_t dtype; };
struct TensorGroupInfo { const TensorInfo* tensors; int64_t numTensors; };
#endif

#ifndef ACLRT_STREAM_DEFINED
#define ACLRT_STREAM_DEFINED
using aclrtStream = void*;
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
inline int aclrtMalloc(void** p, size_t s, int) { if (p) *p = nullptr; (void)s; return 0; }
inline int aclrtFree(void*) { return 0; }
inline int aclrtMemset(void*, size_t, int, size_t) { return 0; }
inline int aclrtMemcpy(void*, size_t, const void*, size_t, int) { return 0; }
inline int aclrtSynchronizeStream(aclrtStream) { return 0; }

#include "R31B-V016-WIDE-TILE-SEED_kernel.asc"
