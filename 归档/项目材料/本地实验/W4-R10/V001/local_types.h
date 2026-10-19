#ifndef TENSOR_GROUP_INFO_DEFINED
#define TENSOR_GROUP_INFO_DEFINED
// Local compile helper only. The judge predefines these types.
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

// GM_ADDR comes from kernel_operator.h.
#ifndef ACLRT_STREAM_DEFINED
#define ACLRT_STREAM_DEFINED
typedef void* aclrtStream;
#endif
