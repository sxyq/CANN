#pragma once

#include <acl/acl.h>
#include <cstdint>

// Local build adapter for the direct-invoke Judge template's predefined ABI.
// This header is not part of the submitted kernel source.
struct TensorInfo {
    const int64_t* shape;
    int64_t numDims;
    int32_t dtype;
};

struct TensorGroupInfo {
    const TensorInfo* tensors;
    int64_t numTensors;
};
