#pragma once

enum class TailCentricDType : uint32_t {
    FP16 = 0,
    BF16 = 1,
    FP32 = 2,
};

struct TailCentricTilingData {
    unsigned int blockNum;
    unsigned int dtype;
    unsigned int outer;
    unsigned int width;
    unsigned int tileElements;
    float epsilon;
    unsigned long long elementsPerBlock;
    unsigned long long tailElements;
};

constexpr uint32_t kTileElements = 2048;
constexpr uint32_t kDoubleBuffer = 2;
