// Local compile helper only. Judge predefines TensorInfo / TensorGroupInfo.
#ifndef MID_X_LOCAL_TYPES_H
#define MID_X_LOCAL_TYPES_H

#ifndef TENSOR_GROUP_INFO_DEFINED
#define TENSOR_GROUP_INFO_DEFINED
struct TensorInfo {
    const long long* shape;
    long long numDims;
    int dtype;
};
struct TensorGroupInfo {
    const TensorInfo* tensors;
    long long numTensors;
};
#endif

#ifndef ACLRT_STREAM_T
#define ACLRT_STREAM_T
typedef void* aclrtStream;
#endif

#endif
