#ifndef ADDR_V002_CORRECTNESS_RUNNER_ABI_H
#define ADDR_V002_CORRECTNESS_RUNNER_ABI_H

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
