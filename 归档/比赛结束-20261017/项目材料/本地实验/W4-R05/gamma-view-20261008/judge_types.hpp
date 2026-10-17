#pragma once
#include <cstdint>
#include <acl/acl.h>
struct TensorInfo { const int64_t* shape; int64_t numDims; int32_t dtype; };
struct TensorGroupInfo { const TensorInfo* tensors; int64_t numTensors; };
using Kernel = void (*)(void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&, void*,
    const TensorGroupInfo&, int64_t, aclrtStream, float);
