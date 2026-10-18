/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <iterator>
#include <vector>

#include "acl/acl.h"
#include "add_custom_proto.h"
#include "ge/ge_api.h"
#include "graph.h"
#include "ops_proto_legacy.h"
#include "tensor.h"
#include "types.h"

using ge::Operator;

namespace {
constexpr uint32_t GRAPH_ID = 0U;
constexpr int64_t BLOCK_NUM = 8;
constexpr int64_t BLOCK_LENGTH = 2048;
constexpr size_t ELEMENT_COUNT = static_cast<size_t>(BLOCK_NUM * BLOCK_LENGTH);
constexpr float X_SCALE = 0.1F;
constexpr float Y_SCALE = 0.2F;
constexpr float CHECK_RTOL = 1e-4F;

// |o>-----------------------------------
// |o>      x    y
// |o>       \  /
// |o>    add_custom0     y
// |o>          \        /
// |o>       add_custom1
// |o>-----------------------------------
// 图构造说明：本例构造两个AddCustom节点，验证GE图中可连续调用通过三尖括号启动的Ascend C核函数
std::unique_ptr<ge::Graph> BuildGraph()
{
    // 构造两个Data节点作为图输入，shape、format和dtype与AddCustom算子声明保持一致
    ge::TensorDesc tensorDesc(ge::Shape({BLOCK_NUM, BLOCK_LENGTH}), ge::FORMAT_ND, ge::DT_FLOAT);
    auto x = ge::op::Data("x");
    x.update_input_desc_x(tensorDesc);
    x.update_output_desc_y(tensorDesc);

    auto y = ge::op::Data("y");
    y.update_input_desc_x(tensorDesc);
    y.update_output_desc_y(tensorDesc);

    // 第一个AddCustom节点计算x + y
    auto add0 = ge::op::AddCustom("add_custom0").set_input_x(x).set_input_y(y);
    add0.update_output_desc_z(tensorDesc);

    // 第二个AddCustom节点继续计算add_custom0 + y，作为整图输出
    auto add1 = ge::op::AddCustom("add_custom1").set_input_x(add0).set_input_y(y);
    add1.update_output_desc_z(tensorDesc);

    std::vector<Operator> inputs{x, y};
    std::vector<Operator> outputs{add1};
    auto graph = std::make_unique<ge::Graph>("TripleChevronAddGraph");
    graph->SetInputs(inputs).SetOutputs(outputs);
    return graph;
}

ge::Tensor BuildInputTensor(float scale)
{
    // 按固定比例构造输入数据，便于后续生成golden结果并校验
    ge::TensorDesc tensorDesc(ge::Shape({BLOCK_NUM, BLOCK_LENGTH}), ge::FORMAT_ND, ge::DT_FLOAT);
    ge::Tensor tensor(tensorDesc);
    std::vector<float> data(ELEMENT_COUNT);
    for (size_t i = 0U; i < data.size(); ++i) {
        data[i] = static_cast<float>(i) * scale;
    }
    if (tensor.SetData(reinterpret_cast<const uint8_t*>(data.data()), data.size() * sizeof(float)) !=
        ge::GRAPH_SUCCESS) {
        std::cerr << "SetData failed." << std::endl;
    }
    return tensor;
}

std::vector<float> BuildGolden()
{
    // golden与图构造规则保持一致，期望结果为(x + y) + y
    std::vector<float> golden(ELEMENT_COUNT);
    for (size_t i = 0U; i < golden.size(); ++i) {
        golden[i] = static_cast<float>(i) * X_SCALE + static_cast<float>(i) * Y_SCALE + static_cast<float>(i) * Y_SCALE;
    }
    return golden;
}

bool CheckOutputTensor(const ge::Tensor& outputTensor)
{
    // 先校验输出shape和数据长度，再逐元素比较计算结果
    const auto tensorDesc = outputTensor.GetTensorDesc();
    const auto dims = tensorDesc.GetShape().GetDims();
    const auto dataType = tensorDesc.GetDataType();
    if ((dims.size() != 2U) || (dims[0] != BLOCK_NUM) || (dims[1] != BLOCK_LENGTH)) {
        std::cerr << "Unexpected output shape." << std::endl;
        return false;
    }
    if (dataType != ge::DT_FLOAT) {
        std::cerr << "Unexpected output dtype: " << static_cast<int>(dataType) << std::endl;
        return false;
    }
    if (outputTensor.GetSize() < ELEMENT_COUNT * sizeof(float)) {
        std::cerr << "Unexpected output data size: " << outputTensor.GetSize() << std::endl;
        return false;
    }

    std::vector<float> output(ELEMENT_COUNT);
    const auto* outputData = reinterpret_cast<const float*>(outputTensor.GetData());
    std::copy(outputData, outputData + ELEMENT_COUNT, output.begin());
    std::vector<float> golden = BuildGolden();
    auto printTensor = [](const std::vector<float>& tensor, const char* name) {
        constexpr size_t maxPrintSize = 20U;
        std::cout << name << ": ";
        std::copy(
            tensor.begin(), tensor.begin() + std::min(tensor.size(), maxPrintSize),
            std::ostream_iterator<float>(std::cout, " "));
        if (tensor.size() > maxPrintSize) {
            std::cout << "...";
        }
        std::cout << std::endl;
    };
    printTensor(output, "Output");
    printTensor(golden, "Golden");

    for (size_t i = 0U; i < ELEMENT_COUNT; ++i) {
        const float diff = std::abs(output[i] - golden[i]);
        const float tolerance = CHECK_RTOL * std::abs(golden[i]);
        if (diff > tolerance) {
            std::cerr << "Unexpected output at index " << i << ", expected " << golden[i] << ", got " << output[i]
                      << ", diff " << diff << ", tolerance " << tolerance << std::endl;
            return false;
        }
    }
    return true;
}
} // namespace

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    // 初始化GE，并指定运行设备和图运行模式
    std::map<ge::AscendString, ge::AscendString> options = {
        {"ge.exec.deviceId", "0"},
        {"ge.exec.precision_mode", "must_keep_origin_dtype"},
        {"ge.graphRunMode", "0"},
    };

    const auto initRet = ge::GEInitialize(options);
    if (initRet != ge::SUCCESS) {
        std::cerr << "GEInitialize failed, ret: " << initRet << std::endl;
        return 1;
    }

    int retCode = 0;
    // 创建GE会话，添加图后通过RunGraph触发两个AddCustom节点依次执行
    ge::Session session(options);
    auto graph = BuildGraph();
    if (graph == nullptr) {
        std::cerr << "BuildGraph failed." << std::endl;
        (void)ge::GEFinalize();
        return 1;
    }

    const auto addGraphRet = session.AddGraph(GRAPH_ID, *graph);
    if (addGraphRet != ge::SUCCESS) {
        std::cerr << "AddGraph failed, ret: " << addGraphRet << std::endl;
        retCode = 1;
    } else {
        // 构造输入Tensor，执行图并检查输出是否符合golden结果
        std::vector<ge::Tensor> inputs{BuildInputTensor(X_SCALE), BuildInputTensor(Y_SCALE)};
        std::vector<ge::Tensor> outputs;
        const auto runRet = session.RunGraph(GRAPH_ID, inputs, outputs);
        if (runRet != ge::SUCCESS) {
            std::cerr << "RunGraph failed, ret: " << runRet << std::endl;
            retCode = 1;
        } else if (outputs.empty()) {
            std::cerr << "RunGraph succeeded but outputs is empty." << std::endl;
            retCode = 1;
        } else if (!CheckOutputTensor(outputs[0])) {
            retCode = 1;
        } else {
            std::cout << "AddCustom GE triple-chevron sample success." << std::endl;
        }
        (void)session.RemoveGraph(GRAPH_ID);
    }

    const auto finalizeRet = ge::GEFinalize();
    if (finalizeRet != ge::SUCCESS) {
        std::cerr << "GEFinalize failed, ret: " << finalizeRet << std::endl;
        return 1;
    }
    return retCode;
}
