#ifndef SMD_V001_LOCAL_TYPES_H
#define SMD_V001_LOCAL_TYPES_H

#include <cstdint>

using aclrtStream = void*;

struct TensorInfo {
    const int64_t* shape;
    int64_t numDims;
    int32_t dtype;
};

struct TensorGroupInfo {
    const TensorInfo* tensors;
    int64_t numTensors;
};

#endif
