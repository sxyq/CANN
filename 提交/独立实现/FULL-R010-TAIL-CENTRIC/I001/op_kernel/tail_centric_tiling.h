#pragma once

#include <cstdint>

enum class TailCentricDType : uint32_t {
    FP16 = 0,
    BF16 = 1,
    FP32 = 2,
};

struct TailCentricTilingData {
    uint32_t blockNum;
    uint32_t dtype;
    uint32_t outer;
    uint32_t width;
    uint32_t tileElements;
    uint32_t epsilonBits;
    uint64_t elementsPerBlock;
    uint64_t tailElements;
};

constexpr uint32_t kTileElements = 2048;
constexpr uint32_t kDoubleBuffer = 2;

