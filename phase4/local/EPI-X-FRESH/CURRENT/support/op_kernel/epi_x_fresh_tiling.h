#pragma once

#include <cstdint>

struct EpiXFreshTilingData {
    uint32_t rows;
    uint32_t d;
    uint32_t blockNum;
    uint32_t rowsPerBlock;
    uint32_t dtype;
    float invD;
    float epsilon;
};
