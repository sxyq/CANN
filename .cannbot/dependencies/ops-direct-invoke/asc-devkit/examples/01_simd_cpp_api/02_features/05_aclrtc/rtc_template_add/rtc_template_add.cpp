/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

/*!
 * \file rtc_template_add.cpp
 * \brief aclrtc的add模板样例，展示模板global函数情况下核函数别名的获取与使用
 */

#include <iostream>
#include <fstream>
#include <vector>

#include "acl/acl.h"
#include "acl/acl_rt.h"
#include "acl/acl_rt_compile.h"
#include "data_utils.h"

#ifndef ACLRTC_NPU_ARCH
#define ACLRTC_NPU_ARCH "dav-2201"
#endif

int main(int argc, char* argv[])
{
    const char* src = R""""(
#include "kernel_operator.h"
constexpr int32_t TOTAL_LENGTH = 1024;

class KernelAdd {
public:
    __aicore__ inline KernelAdd() {}
    __aicore__ inline void Init(GM_ADDR x, GM_ADDR y, GM_ADDR z)
    {
        xGm.SetGlobalBuffer((__gm__ float *)x, TOTAL_LENGTH);
        yGm.SetGlobalBuffer((__gm__ float *)y, TOTAL_LENGTH);
        zGm.SetGlobalBuffer((__gm__ float *)z, TOTAL_LENGTH);
    }
    __aicore__ inline void Process()
    {
        AscendC::LocalMemAllocator<AscendC::Hardware::UB> ubAllocator;
        AscendC::LocalTensor<float> xLocal = ubAllocator.Alloc<float, TOTAL_LENGTH>();
        AscendC::LocalTensor<float> yLocal = ubAllocator.Alloc<float, TOTAL_LENGTH>();
        AscendC::LocalTensor<float> zLocal = ubAllocator.Alloc<float, TOTAL_LENGTH>();

        AscendC::DataCopy(xLocal, xGm, TOTAL_LENGTH);
        AscendC::DataCopy(yLocal, yGm, TOTAL_LENGTH);
        // MTE2→V同步：确保DataCopy完成后再计算
        AscendC::SetFlag<AscendC::HardEvent::MTE2_V>(EVENT_ID0);
        AscendC::WaitFlag<AscendC::HardEvent::MTE2_V>(EVENT_ID0);

        AscendC::Add(zLocal, xLocal, yLocal, TOTAL_LENGTH);
        // V→MTE3同步：确保Add完成后再将结果拷出
        AscendC::SetFlag<AscendC::HardEvent::V_MTE3>(EVENT_ID0);
        AscendC::WaitFlag<AscendC::HardEvent::V_MTE3>(EVENT_ID0);

        AscendC::DataCopy(zGm, zLocal, TOTAL_LENGTH);
    }

private:
    AscendC::GlobalTensor<float> xGm;
    AscendC::GlobalTensor<float> yGm;
    AscendC::GlobalTensor<float> zGm;
};

namespace Kernel {
template<typename T>
__global__ __vector__ void add_custom(GM_ADDR x, GM_ADDR y, GM_ADDR z)
{
    AscendC::InitSocState();
    KernelAdd op;
    op.Init(x, y, z);
    op.Process();
    AscendC::PipeBarrier<PIPE_ALL>();
}
}
)"""";

    aclrtcProg prog;
    ASCENDC_CHECK(aclrtcCreateProg(&prog, src, "add_custom", 0, nullptr, nullptr));

    const char* kernelNameExpr = "Kernel::add_custom<float>";
    ASCENDC_CHECK(aclrtcAddNameExpr(prog, kernelNameExpr));

    const char* options[] = {
        "--npu-arch=" ACLRTC_NPU_ARCH,
    };

    int numOptions = sizeof(options) / sizeof(options[0]);
    ASCENDC_CHECK(aclrtcCompileProg(prog, numOptions, options));

    size_t binDataSizeRet;
    ASCENDC_CHECK(aclrtcGetBinDataSize(prog, &binDataSizeRet));

    std::vector<char> deviceELF(binDataSizeRet);
    ASCENDC_CHECK(aclrtcGetBinData(prog, deviceELF.data()));

    const char* manglingName = "";
    ASCENDC_CHECK(aclrtcGetLoweredName(prog, kernelNameExpr, &manglingName));

    uint32_t numBlocks = 1;
    uint32_t dataLen = 1024; // TOTAL_LENGTH
    size_t inputByteSize = dataLen * sizeof(float);
    size_t outputByteSize = dataLen * sizeof(float);

    ASCENDC_CHECK(aclInit(nullptr));
    int32_t deviceId = 0;
    ASCENDC_CHECK(aclrtSetDevice(deviceId));
    aclrtStream stream = nullptr;
    ASCENDC_CHECK(aclrtCreateStream(&stream));

    float *xHost, *yHost, *zHost, *golden;
    uint8_t *xDevice, *yDevice, *zDevice;

    ASCENDC_CHECK(aclrtMallocHost((void**)(&xHost), inputByteSize));
    ASCENDC_CHECK(aclrtMallocHost((void**)(&yHost), inputByteSize));
    ASCENDC_CHECK(aclrtMallocHost((void**)(&zHost), outputByteSize));
    ASCENDC_CHECK(aclrtMallocHost((void**)(&golden), outputByteSize));
    ASCENDC_CHECK(aclrtMalloc((void**)&xDevice, inputByteSize, ACL_MEM_MALLOC_HUGE_FIRST));
    ASCENDC_CHECK(aclrtMalloc((void**)&yDevice, inputByteSize, ACL_MEM_MALLOC_HUGE_FIRST));
    ASCENDC_CHECK(aclrtMalloc((void**)&zDevice, outputByteSize, ACL_MEM_MALLOC_HUGE_FIRST));

    GenTestData(xHost, yHost, golden, dataLen);

    ASCENDC_CHECK(aclrtMemcpy(xDevice, inputByteSize, xHost, inputByteSize, ACL_MEMCPY_HOST_TO_DEVICE));
    ASCENDC_CHECK(aclrtMemcpy(yDevice, inputByteSize, yHost, inputByteSize, ACL_MEMCPY_HOST_TO_DEVICE));

    aclrtBinHandle binHandle = nullptr;
    aclrtBinaryLoadOptions loadOption;
    loadOption.numOpt = 1;
    aclrtBinaryLoadOption option;
    option.type = ACL_RT_BINARY_LOAD_OPT_MAGIC;
    option.value.magic = ACL_RT_BINARY_MAGIC_ELF_AICORE;
    loadOption.options = &option;
    ASCENDC_CHECK(aclrtBinaryLoadFromData(deviceELF.data(), binDataSizeRet, &loadOption, &binHandle));
    aclrtFuncHandle funcHandle = nullptr;

    ASCENDC_CHECK(aclrtBinaryGetFunction(binHandle, manglingName, &funcHandle));

    void* kernelArgs[] = {
        static_cast<void*>(&xDevice),
        static_cast<void*>(&yDevice),
        static_cast<void*>(&zDevice),
    };
    ASCENDC_CHECK(aclrtLaunchKernelWithArgsArray(funcHandle, numBlocks, stream, nullptr, kernelArgs));

    ASCENDC_CHECK(aclrtSynchronizeStream(stream));
    ASCENDC_CHECK(aclrtMemcpy(zHost, outputByteSize, zDevice, outputByteSize, ACL_MEMCPY_DEVICE_TO_HOST));

    // 精度校验（C++内完成，无需Python脚本）
    if (VerifyResult(zHost, golden, dataLen)) {
        printf("test pass!\n");
    } else {
        printf("[ERROR] result error\n");
        return 1;
    }

    ASCENDC_CHECK(aclrtBinaryUnLoad(binHandle));
    ASCENDC_CHECK(aclrtFree(xDevice));
    ASCENDC_CHECK(aclrtFree(yDevice));
    ASCENDC_CHECK(aclrtFree(zDevice));
    ASCENDC_CHECK(aclrtFreeHost(golden));
    ASCENDC_CHECK(aclrtFreeHost(xHost));
    ASCENDC_CHECK(aclrtFreeHost(yHost));
    ASCENDC_CHECK(aclrtFreeHost(zHost));
    ASCENDC_CHECK(aclrtDestroyStream(stream));
    ASCENDC_CHECK(aclrtResetDevice(deviceId));
    ASCENDC_CHECK(aclFinalize());

    ASCENDC_CHECK(aclrtcDestroyProg(&prog));

    return 0;
}
