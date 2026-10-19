// Local compile helper only. Must match judge TensorInfo / TensorGroupInfo layout.
#pragma once

typedef signed long i001_i64;
typedef signed int i001_i32;

#ifndef ACLRT_STREAM_T
#define ACLRT_STREAM_T
typedef void* aclrtStream;
#endif

#ifndef ACL_SUCCESS
#define ACL_SUCCESS 0
#endif
#ifndef ACL_MEM_MALLOC_HUGE_FIRST
#define ACL_MEM_MALLOC_HUGE_FIRST 1
#endif

extern "C" int aclrtMalloc(void** devPtr, unsigned long size, int kind);
extern "C" int aclrtFree(void* devPtr);
extern "C" int aclrtMemset(void* devPtr, unsigned long destMax, int value, unsigned long count);
extern "C" int aclrtMemcpy(void* dst, unsigned long destMax, const void* src,
                           unsigned long count, int kind);
extern "C" int aclrtSynchronizeStream(aclrtStream stream);
#ifndef ACL_MEMCPY_HOST_TO_DEVICE
#define ACL_MEMCPY_HOST_TO_DEVICE 1
#endif

struct TensorInfo {
    const i001_i64* shape;
    i001_i64 numDims;
    i001_i32 dtype;
};

struct TensorGroupInfo {
    const TensorInfo* tensors;
    i001_i64 numTensors;
};
