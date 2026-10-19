#ifndef EPI_ARITH_V001_LOCAL_ABI_SHIM_H
#define EPI_ARITH_V001_LOCAL_ABI_SHIM_H

#include <acl/acl.h>
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

#ifndef EPI_ASC_KERNEL_COMPILE
using GM_ADDR = void*;
#endif

#endif
