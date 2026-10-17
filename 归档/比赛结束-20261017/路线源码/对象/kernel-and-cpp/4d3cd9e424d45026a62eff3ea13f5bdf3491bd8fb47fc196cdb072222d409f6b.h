#ifndef SYNC_TOPOLOGY_V001_LOCAL_TENSOR_METADATA_H
#define SYNC_TOPOLOGY_V001_LOCAL_TENSOR_METADATA_H

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
