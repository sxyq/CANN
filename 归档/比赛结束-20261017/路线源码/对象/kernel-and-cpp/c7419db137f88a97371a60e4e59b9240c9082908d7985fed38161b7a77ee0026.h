#pragma once

#include <stdint.h>

constexpr uint32_t RNB_DOUBLE_BUFFER = 1;
constexpr float RNB_EPSILON = 1.0e-5f;

struct AddRmsNormBiasTilingData {
    uint32_t blockNum;
    uint32_t outer;
    uint32_t dim;
    uint32_t alignedDim;
    uint32_t rowsPerBlock;
    uint32_t tailRows;
    float epsilon;
};
