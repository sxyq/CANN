#include <cstdint>
#include <iostream>

#include "graph/tensor.h"
#include "tiling/normalization/rmsnorm_tiling.h"

namespace {

bool BuildTiling(const uint32_t rows, const uint32_t width, const uint32_t type_size,
    const uint32_t tmp_bytes, RmsNormTiling& out)
{
    ge::Shape shape({1, rows, width});
    return AscendC::GetRmsNormTilingInfo(shape, shape, tmp_bytes, type_size, out, false);
}

void Probe(const uint32_t rows, const uint32_t width, const uint32_t type_size)
{
    ge::Shape shape({1, rows, width});
    uint32_t max_tmp = 0;
    uint32_t min_tmp = 0;
    const bool range_ok = AscendC::GetRmsNormMaxMinTmpSize(shape, type_size, max_tmp, min_tmp, false);
    RmsNormTiling tiling{};
    const bool tile_ok = range_ok && BuildTiling(rows, width, type_size, min_tmp, tiling);
    std::cout << "rows=" << rows << " width=" << width << " bytes=" << type_size
              << " range=" << range_ok << " min=" << min_tmp << " max=" << max_tmp
              << " tile=" << tile_ok;
    if (tile_ok) {
        std::cout << " h=" << tiling.hLength << " originalH=" << tiling.originalHLength
                  << " mainBs=" << tiling.mainBsLength << " mainBsh=" << tiling.mainBshLength
                  << " loop=" << tiling.loopRound << " tail=" << tiling.tailBsLength;
    }
    std::cout << '\n';
}

}  // namespace

int main()
{
    for (const uint32_t width : {64U, 96U, 128U, 1024U, 4096U, 8192U, 32768U}) {
        Probe(1, width, sizeof(float));
        Probe(4, width, sizeof(float));
        Probe(1, width, 2U);
    }
    return 0;
}
