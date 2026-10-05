#ifndef WIDE_X_FRESH4_LOCAL_ABI_SHIM_H
#define WIDE_X_FRESH4_LOCAL_ABI_SHIM_H

#include <cstdint>

struct TensorInfo {
    const int64_t* shape;
    int64_t numDims;
    int32_t dtype;
};

struct TensorGroupInfo {
    const TensorInfo* tensors;
    int64_t numTensors;
};

#ifndef GM_ADDR
using GM_ADDR = void*;
#endif

#endif
