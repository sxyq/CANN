#pragma once

#include <cstdint>

struct TensorInfo {
    const std::int64_t* shape;
    std::int64_t numDims;
    std::int32_t dtype;
};

struct TensorGroupInfo {
    const TensorInfo* tensors;
    std::int64_t numTensors;
};

using KernelGmAddr = unsigned char*;
