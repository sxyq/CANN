// Local compile helper only. Judge predefines TensorInfo / TensorGroupInfo.
#ifndef MODE_X_LOCAL_TYPES_H
#define MODE_X_LOCAL_TYPES_H

#include <cstdint>

#ifndef TENSOR_GROUP_INFO_DEFINED
#define TENSOR_GROUP_INFO_DEFINED
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

#endif
