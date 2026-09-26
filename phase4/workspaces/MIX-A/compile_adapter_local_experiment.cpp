#include "compile_adapter_local_experiment.asc"
#include <cstdint>

using aclrtStream = void*;

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

#include "MIX-A-local-mode-replacement.asc"

int main()
{
    return 0;
}
