#pragma once

#include <stdint.h>

struct RowCopyTiling {
    uint32_t rows;
    uint32_t cols;
};

constexpr uint32_t kMinCols = 256;
constexpr uint32_t kMaxCols = 8192;
constexpr uint32_t kRowsPerBlock = 2;
