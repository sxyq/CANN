#ifndef H001_COMPILE_ADAPTER_HPP
#define H001_COMPILE_ADAPTER_HPP

#include <cstdint>

// Local compile-only stand-ins for judge-provided ABI types.
// The submission source must NOT redefine these.
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
