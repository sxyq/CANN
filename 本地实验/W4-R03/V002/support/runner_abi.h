#ifndef W4_R03_V002_RUNNER_ABI_H
#define W4_R03_V002_RUNNER_ABI_H

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

#endif
