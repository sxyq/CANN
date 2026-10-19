#pragma once

#include <cstdint>

// The scalar fields are deliberately explicit: the host launcher can build this
// record without depending on an operator registration or generated tiling code.
struct AddRmsNormBiasTilingData {
    uint64_t rows;
    uint64_t d;
    uint64_t firstRow;
    uint64_t rowsThisCore;
    uint32_t blockNum;
    uint32_t dtype;          // 0: fp16, 1: bf16, 2: fp32
    uint32_t chunkElements;  // valid elements in one streamed column chunk
    float epsilon;
    float invD;
};

constexpr uint32_t kA001HotPathD = 1024;
constexpr uint32_t kA001Fp32Chunk = 1024;
constexpr uint32_t kA001HalfChunk = 2048;
constexpr uint32_t kA001DoubleBuffer = 2;
