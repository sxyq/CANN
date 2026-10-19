#ifndef DTYPE_LOWP_FASTPATH_V001_LOCAL_TYPES_H
#define DTYPE_LOWP_FASTPATH_V001_LOCAL_TYPES_H

#include <cstdint>

struct TensorInfo {
    const int64_t *shape;
    int64_t numDims;
    int32_t dtype;
};

struct TensorGroupInfo {
    const TensorInfo *tensors;
    int64_t numTensors;
};

typedef void *aclrtStream;

#endif
