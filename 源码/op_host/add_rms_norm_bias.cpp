/**
 * AddRmsNormBias Host 侧实现：算子原型注册 / InferShape / Tiling 下发
 *
 * 说明：本文件按 CANN 8.x/9.0.0 msopgen（新版 gert 风格）模板编写。
 * 本机无 CANN 环境，未编译验证。真机落地时优先以 msopgen 生成模板为骨架，
 * 替换同名文件并核对以下接口在当前 CANN 版本的签名：
 *   - gert::InferShapeContext / gert::TilingContext（老版本为 ge::Operator / ge::TilingContext）
 *   - REG_OP / OP_END_FACTORY_REG / REGISTER_TILING_FUNC 宏
 *   - TILING_DATA_DEF 宏（kernel 侧当前使用 GET_TILING_DATA_WITH_STRUCT）
 */

#include "add_rms_norm_bias_tiling.h"
#include "register/op_def_registry.h"

namespace ge {
// 算子原型：与 add_rms_norm_bias.json 一致
REG_OP(AddRmsNormBias)
    .INPUT(x, TensorType({DT_FLOAT16, DT_BF16, DT_FLOAT}))
    .INPUT(residual, TensorType({DT_FLOAT16, DT_BF16, DT_FLOAT}))
    .INPUT(gamma, TensorType({DT_FLOAT16, DT_BF16, DT_FLOAT}))
    .INPUT(bias, TensorType({DT_FLOAT16, DT_BF16, DT_FLOAT}))
    .OUTPUT(output, TensorType({DT_FLOAT16, DT_BF16, DT_FLOAT}))
    .ATTR(epsilon, Float, 1e-5)
    .OP_END_FACTORY_REG(AddRmsNormBias);
}  // namespace ge

namespace ops {

// ---- dtype 编码（与 op_kernel 保持一致）----
namespace {
constexpr uint32_t DT_FP16 = 1;
constexpr uint32_t DT_BF16 = 2;
constexpr uint32_t DT_FP32 = 3;
uint32_t DtypeCode(gert::DataType dt)
{
    switch (dt) {
        case ge::DT_FLOAT16: return DT_FP16;
        case ge::DT_BF16: return DT_BF16;
        case ge::DT_FLOAT: return DT_FP32;
        default: return DT_FP32;
    }
}
}  // namespace

// ---- InferShape：输出 shape 与 x 一致；校验 residual 与 x 相同、gamma/bias 为 (D,) ----
static graphStatus AddRmsNormBiasInferShape(gert::InferShapeContext* context)
{
    auto xShape = context->GetInputShape(0);
    auto rShape = context->GetInputShape(1);
    auto gShape = context->GetInputShape(2);
    auto bShape = context->GetInputShape(3);
    if (xShape == nullptr || rShape == nullptr || gShape == nullptr || bShape == nullptr) {
        return GRAPH_FAILED;
    }
    // 形状约束：x 与 residual 全等；gamma/bias 为一维且长度等于 x 最后一维
    if (rShape->GetDimNum() != xShape->GetDimNum()) {
        return GRAPH_FAILED;
    }
    for (size_t i = 0; i < xShape->GetDimNum(); ++i) {
        if (rShape->GetDim(i) != xShape->GetDim(i)) {
            return GRAPH_FAILED;
        }
    }
    if (xShape->GetDimNum() < 2 || xShape->GetDimNum() > 4) {
        return GRAPH_FAILED;  // 题面要求 2D/3D/4D
    }
    int64_t D = xShape->GetDim(xShape->GetDimNum() - 1);
    if (gShape->GetDimNum() != 1 || bShape->GetDimNum() != 1 ||
        gShape->GetDim(0) != D || bShape->GetDim(0) != D) {
        return GRAPH_FAILED;
    }
    // 输出与 x 同 shape
    context->SetOutputShape(0, *xShape);
    return GRAPH_SUCCESS;
}
REGISTER_INFER_FUNC("AddRmsNormBias", AddRmsNormBiasInferShape);
}  // namespace ops

namespace optiling {

static graphStatus AddRmsNormBiasTilingFunc(gert::TilingContext* context)
{
    if (context == nullptr) {
        return GRAPH_FAILED;
    }
    // 从输入张量推导 outer/D/dtype
    const gert::Shape* xShape = context->GetInputShape(0);
    const auto* xDesc = context->GetInputDesc(0);
    if (xShape == nullptr || xDesc == nullptr) {
        return GRAPH_FAILED;
    }
    auto dtype = xDesc->GetDataType();
    uint32_t dimNum = xShape->GetDimNum();
    if (dimNum < 2 || dimNum > 4) {
        return GRAPH_FAILED;
    }
    int64_t D = xShape->GetDim(dimNum - 1);
    int64_t outer = 1;
    for (uint32_t i = 0; i + 1 < dimNum; ++i) {
        outer *= xShape->GetDim(i);
    }
    double eps = 1e-5;
    if (context->GetAttrs() != nullptr) {
        // RuntimeAttrs uses the attribute position from the prototype, not its name.
        const float* epsilon = context->GetAttrs()->GetFloat(0);
        if (epsilon != nullptr) {
            eps = *epsilon;
        }
    }

    // 写入 Tiling 数据
    AddRmsNormBiasTilingData tiling;
    tiling.set_outer(static_cast<uint32_t>(outer));
    tiling.set_D(static_cast<uint32_t>(D));
    tiling.set_epsilon(static_cast<float>(eps));
    tiling.set_dtype(DtypeCode(dtype));
    context->SetTilingData(&tiling, sizeof(tiling));

    // 核数：行数多于核数时全核参与（engine 侧 GetCoreNum 提供可用核数）
    uint32_t coreNum = context->GetCoreNum() > 0 ? context->GetCoreNum() : 1;
    uint32_t blockDim = static_cast<uint32_t>(outer) < coreNum
                            ? (static_cast<uint32_t>(outer) > 0 ? static_cast<uint32_t>(outer) : 1)
                            : coreNum;
    context->SetBlockDim(blockDim);
    return GRAPH_SUCCESS;
}

namespace {
REGISTER_TILING_FUNC(AddRmsNormBias, AddRmsNormBiasTilingFunc);
}  // namespace
}  // namespace optiling
