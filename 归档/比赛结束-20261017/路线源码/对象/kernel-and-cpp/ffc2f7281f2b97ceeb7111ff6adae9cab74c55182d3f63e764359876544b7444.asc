#include <cstdint>
#include <acl/acl_rt.h>

// Minimal host metadata declarations supplied by the direct-invoke wrapper.
struct TensorInfo {
    const int64_t* shape;
    int64_t numDims;
    int32_t dtype;
};

struct TensorGroupInfo {
    const TensorInfo* tensors;
    int64_t numTensors;
};

#include "submission.asc"
