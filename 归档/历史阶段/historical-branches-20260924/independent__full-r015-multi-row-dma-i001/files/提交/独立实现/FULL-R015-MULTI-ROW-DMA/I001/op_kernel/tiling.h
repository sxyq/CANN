#pragma once

#include <cstdint>

constexpr uint32_t MULTI_ROW_TILE = 8;

struct AddRmsNormBiasTilingData {
    int32_t rows;
    int32_t d;
    int32_t rowsPerTile;
    int32_t dAligned;
    float epsilon;
};
