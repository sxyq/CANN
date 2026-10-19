// Local compile helper only. Must match judge TensorInfo / TensorGroupInfo layout.
#pragma once

typedef signed long i001_i64;
typedef signed int i001_i32;

#ifndef ACLRT_STREAM_T
#define ACLRT_STREAM_T
typedef void* aclrtStream;
#endif

struct TensorInfo {
    const i001_i64* shape;
    i001_i64 numDims;
    i001_i32 dtype;
};

struct TensorGroupInfo {
    const TensorInfo* tensors;
    i001_i64 numTensors;
};
