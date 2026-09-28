#pragma once

#include <stdint.h>

enum class AddrRmsNormBiasDType : uint32_t {
    FP16 = 0,
    BF16 = 1,
    FP32 = 2,
};

// One row is visited in bounded chunks so a large D does not consume all UB.
constexpr uint32_t ADDRMSNORMBIAS_TILE_D = 1024;
constexpr uint32_t ADDRMSNORMBIAS_ALIGN_BYTES = 32;

struct AddrRmsNormBiasTilingData {
    uint32_t blockNum;
    uint32_t outer;
    uint32_t dim;
    uint32_t tileDim;
    uint32_t tileDimAligned;
    uint32_t dtype;
    uint64_t rowsPerBlock;
    uint64_t tailRows;
    float epsilon;
    float invDim;
};
