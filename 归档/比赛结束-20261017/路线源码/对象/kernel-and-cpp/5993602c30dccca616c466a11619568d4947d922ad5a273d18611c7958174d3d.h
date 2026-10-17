#ifndef ADD_RMS_NORM_BIAS_TILING_H
#define ADD_RMS_NORM_BIAS_TILING_H

#include "tiling/tiling_api.h"

namespace optiling {

// 下发到 Kernel 的 Tiling 数据。
// 字段说明：
//   outer    : 归约之外所有维度的元素积（batch*seq*heads），即“行数”
//   D        : 最后一维（归一化维）长度
//   epsilon  : 平滑项（题面默认 1e-5）
//   dtype    : 元素类型编码，1=float16, 2=bfloat16, 3=float32
//              （与 input_desc 顺序一致，由 Host 侧从张量 dtype 推断）
BEGIN_TILING_DATA_DEF(AddRmsNormBiasTilingData)
    TILING_DATA_FIELD_DEF(uint32_t, outer);
    TILING_DATA_FIELD_DEF(uint32_t, D);
    TILING_DATA_FIELD_DEF(float, epsilon);
    TILING_DATA_FIELD_DEF(uint32_t, dtype);
END_TILING_DATA_DEF;

}  // namespace optiling

#endif  // ADD_RMS_NORM_BIAS_TILING_H
