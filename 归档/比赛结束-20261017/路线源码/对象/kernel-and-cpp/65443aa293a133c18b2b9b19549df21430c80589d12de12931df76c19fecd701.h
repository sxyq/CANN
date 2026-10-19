#pragma once

#include <cstdint>

using GM_ADDR = void*;
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
