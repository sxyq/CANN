#ifndef CROSSROW_V001_RUNNER_ABI_H
#define CROSSROW_V001_RUNNER_ABI_H

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
