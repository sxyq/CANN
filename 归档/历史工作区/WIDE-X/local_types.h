#ifndef TENSOR_GROUP_INFO_DEFINED
#define TENSOR_GROUP_INFO_DEFINED
// Local compile helper only. Judge predefines these types.
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

// Judge also predefines aclrtStream / GM_ADDR. Local shims provide a stub.
#ifndef ACLRTSTREAM_DEFINED_LOCAL
#define ACLRTSTREAM_DEFINED_LOCAL
struct aclrtStream_st;
typedef struct aclrtStream_st* aclrtStream;
#endif

