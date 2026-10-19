// Local compile helper only. The online submission must NOT redefine these types.
// Judge notes: dtype encoding 0=FP32, 1=FP16, 2=BF16.
#pragma once

typedef signed long i001_i64;
typedef signed int i001_i32;

struct TensorInfo {
    const i001_i64* shape;
    i001_i64 numDims;
    i001_i32 dtype;
};

struct TensorGroupInfo {
    const TensorInfo* tensors;
    i001_i64 numTensors;
};
