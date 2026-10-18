/**
 * Copyright (c) 2025 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <vector>

#include "aclnn/aclnn_base.h"
#include "aclnn/acl_meta.h"
#include "acl/acl_rt.h"
#include "aclnn_add_custom.h"

#define CHECK_ACL(expr)                                                                                 \
    do {                                                                                                \
        auto __ret = (expr);                                                                            \
        int32_t __code = static_cast<int32_t>(__ret);                                                   \
        if (__code != 0) {                                                                              \
            fprintf(stderr, "[ERROR] %s failed at %s:%d, ret=%d\n", #expr, __FILE__, __LINE__, __code); \
        }                                                                                               \
    } while (0)

bool ReadFile(const char* filePath, void* data, size_t size)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file || !file.read(static_cast<char*>(data), size)) {
        fprintf(stderr, "[ERROR] Failed to read %s\n", filePath);
        return false;
    }
    return true;
}

bool VerifyResult(const std::vector<aclFloat16>& outputData, const std::vector<aclFloat16>& goldenData)
{
    if (outputData.size() != goldenData.size()) {
        return false;
    }

    constexpr float tolerance = 1e-3F;
    return std::equal(
        outputData.begin(), outputData.end(), goldenData.begin(), [](aclFloat16 output, aclFloat16 golden) {
            const float outputValue = aclFloat16ToFloat(output);
            const float goldenValue = aclFloat16ToFloat(golden);
            return std::fabs(outputValue - goldenValue) <= tolerance;
        });
}

int32_t main(int32_t argc, char** argv)
{
    const std::vector<int64_t> shape = {8, 2048};
    const int64_t elementCount = shape[0] * shape[1];
    const size_t bufferSize = elementCount * sizeof(aclFloat16);
    std::vector<aclFloat16> input0HostData(elementCount);
    std::vector<aclFloat16> input1HostData(elementCount);
    std::vector<aclFloat16> output0HostData(elementCount, aclFloatToFloat16(0.0));
    std::vector<aclFloat16> goldenData(elementCount);
    if (!ReadFile("input/input0.bin", input0HostData.data(), bufferSize) ||
        !ReadFile("input/input1.bin", input1HostData.data(), bufferSize) ||
        !ReadFile("output/golden.bin", goldenData.data(), bufferSize)) {
        return 1;
    }

    const int32_t deviceId = 0;
    aclrtStream stream = nullptr;
    CHECK_ACL(aclnnInit(nullptr));
    CHECK_ACL(aclrtSetDevice(deviceId));
    CHECK_ACL(aclrtCreateStream(&stream));

    void* input0DeviceMem = nullptr;
    CHECK_ACL(aclrtMalloc(&input0DeviceMem, bufferSize, ACL_MEM_MALLOC_HUGE_FIRST));
    aclTensor* input0 = aclCreateTensor(
        shape.data(), shape.size(), ACL_FLOAT16, nullptr, 0, ACL_FORMAT_ND, shape.data(), shape.size(),
        input0DeviceMem);

    void* input1DeviceMem = nullptr;
    CHECK_ACL(aclrtMalloc(&input1DeviceMem, bufferSize, ACL_MEM_MALLOC_HUGE_FIRST));
    aclTensor* input1 = aclCreateTensor(
        shape.data(), shape.size(), ACL_FLOAT16, nullptr, 0, ACL_FORMAT_ND, shape.data(), shape.size(),
        input1DeviceMem);

    void* output0DeviceMem = nullptr;
    CHECK_ACL(aclrtMalloc(&output0DeviceMem, bufferSize, ACL_MEM_MALLOC_HUGE_FIRST));
    aclTensor* output0 = aclCreateTensor(
        shape.data(), shape.size(), ACL_FLOAT16, nullptr, 0, ACL_FORMAT_ND, shape.data(), shape.size(),
        output0DeviceMem);

    CHECK_ACL(aclrtMemcpy(input0DeviceMem, bufferSize, input0HostData.data(), bufferSize, ACL_MEMCPY_HOST_TO_DEVICE));
    CHECK_ACL(aclrtMemcpy(input1DeviceMem, bufferSize, input1HostData.data(), bufferSize, ACL_MEMCPY_HOST_TO_DEVICE));

    uint64_t workspaceSize = 0;
    aclOpExecutor* executor = nullptr;
    CHECK_ACL(aclnnAddCustomGetWorkspaceSize(input0, input1, output0, &workspaceSize, &executor));
    void* workspaceDeviceMem = nullptr;
    if (workspaceSize > 0) {
        CHECK_ACL(aclrtMalloc(&workspaceDeviceMem, workspaceSize, ACL_MEM_MALLOC_HUGE_FIRST));
    }
    CHECK_ACL(aclnnAddCustom(workspaceDeviceMem, workspaceSize, executor, stream));
    CHECK_ACL(aclrtSynchronizeStream(stream));
    CHECK_ACL(aclrtMemcpy(output0HostData.data(), bufferSize, output0DeviceMem, bufferSize, ACL_MEMCPY_DEVICE_TO_HOST));

    printf("result is:\n");
    const int64_t previewCount = std::min<int64_t>(elementCount, 10);
    for (int64_t i = 0; i < previewCount; i++) {
        printf("%.1f ", aclFloat16ToFloat(output0HostData[i]));
    }
    printf("\ntest %s\n", VerifyResult(output0HostData, goldenData) ? "pass" : "failed");

    aclDestroyTensor(input0);
    aclDestroyTensor(input1);
    aclDestroyTensor(output0);
    CHECK_ACL(aclrtFree(input0DeviceMem));
    CHECK_ACL(aclrtFree(input1DeviceMem));
    CHECK_ACL(aclrtFree(output0DeviceMem));
    if (workspaceSize > 0) {
        CHECK_ACL(aclrtFree(workspaceDeviceMem));
    }
    CHECK_ACL(aclrtDestroyStream(stream));
    CHECK_ACL(aclrtResetDevice(deviceId));
    CHECK_ACL(aclnnFinalize());
    return 0;
}
