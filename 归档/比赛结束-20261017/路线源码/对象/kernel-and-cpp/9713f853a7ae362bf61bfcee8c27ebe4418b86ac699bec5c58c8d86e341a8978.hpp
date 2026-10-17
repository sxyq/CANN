#ifndef H001_COMPILE_ADAPTER_HPP
#define H001_COMPILE_ADAPTER_HPP

// Local-only stand-ins for types the judge predefines before including kernel.asc.
// Mirrors npu_kernel_dev / judge.asc field layout. Never submitted.

#include <cstdint>

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

#endif
