#ifndef TINY_MINIMAL_V003_PAIRED_RUNNER_ABI_H
#define TINY_MINIMAL_V003_PAIRED_RUNNER_ABI_H

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
