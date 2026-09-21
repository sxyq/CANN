#pragma once

#include <cstdint>

#include "kernel_tiling/kernel_tiling.h"

// E001 keeps the runtime record deliberately small and plain-old-data so that
// the direct-invoke host and device paths use the same byte layout.
struct E001TilingData {
    uint32_t rank;
    uint32_t rows;
    uint32_t width;
    uint32_t block_num;
    uint32_t rows_per_block;
    uint32_t rows_last_block;
    uint32_t tile_rows;
    uint32_t dtype_code;
    uint32_t hot_path;
    uint32_t rms_tmp_bytes;
    uint32_t chunk_elements;
    uint32_t reserved;
    uint64_t workspace_bytes;
    float epsilon;
    float reciprocal_width;
    RmsNormTiling rms_tiling;
};

static_assert(sizeof(E001TilingData) % 8 == 0, "tiling record must be 8-byte aligned");

constexpr uint32_t E001_DAV2201_VEC_WORKSPACE = 184U * 1024U;
// The primitive is invoked once per UB row.  Keeping the record explicit
// avoids deriving a multi-row primitive tile from the global row count.
constexpr uint32_t E001_TILE_ROWS_DEFAULT = 1;
constexpr uint32_t E001_FALLBACK_CHUNK_ELEMENTS = 2048;
