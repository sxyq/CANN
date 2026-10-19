#ifndef SERVER_AUDIT_JUDGE_ABI_H
#define SERVER_AUDIT_JUDGE_ABI_H
#include <cstdint>
#include <cstddef>
/* Prefer real ACL headers when present; they define aclrtStream / aclrtMalloc. */
#if defined(__has_include)
#  if __has_include("acl/acl.h")
#    include "acl/acl.h"
#  elif __has_include("acl/acl_base.h")
#    include "acl/acl_base.h"
#  endif
#endif
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
#ifndef GM_ADDR
using GM_ADDR = void*;
#endif
#ifndef ACLRT_STREAM_T
#define ACLRT_STREAM_T
#if !defined(ACLRT_STREAM_DEFINED) && !defined(__ACLRT_STREAM_TYPE_DEFINED)
/* fallback only if ACL headers were not included */
#endif
#endif
#endif
