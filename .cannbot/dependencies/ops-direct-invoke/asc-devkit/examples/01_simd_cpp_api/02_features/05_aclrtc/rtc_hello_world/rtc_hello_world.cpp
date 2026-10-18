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
 * \file rtc_hello_world.cpp
 * \brief 简单的hello world样例，展示aclrtc的基本用法
 */

#include <cstdio>
#include <vector>
#include "acl/acl.h"
#include "acl/acl_rt_compile.h" // 使用aclrtc接口需要包含的头文件

#ifndef ACL_RTC_NPU_ARCH
#define ACL_RTC_NPU_ARCH "dav-2201"
#endif

#define ASCENDC_CHECK(expr)                                                                                     \
    do {                                                                                                        \
        aclError ret = (expr);                                                                                  \
        if (ret != ACL_SUCCESS) {                                                                               \
            fprintf(stderr, "Ascend Error: %s:%d code=%d %s\n", __FILE__, __LINE__, ret, aclGetRecentErrMsg()); \
            return ret;                                                                                         \
        }                                                                                                       \
    } while (0)

const char* src = R""""(
#include "utils/debug/asc_printf.h"
extern "C" __global__ __vector__ void hello_world()
{
    printf("Hello World!!!\n");
}
)"""";

int main(int argc, char* argv[])
{
    aclrtcProg prog;
    ASCENDC_CHECK(aclrtcCreateProg(&prog, src, "hello_world.asc", 0, nullptr, nullptr));

    // aclrtc流程，传入毕昇编译器的编译选项，调用aclrtcCompileProg进行编译
    const char* options[] = {
        "--npu-arch=" ACL_RTC_NPU_ARCH,
    };
    int numOptions = sizeof(options) / sizeof(options[0]);
    aclError ret = aclrtcCompileProg(prog, 1, options);
    if (ret != ACL_SUCCESS) {
        size_t size = 0;
        (void)aclrtcGetCompileLogSize(prog, &size);
        char log[size] = {0};
        (void)aclrtcGetCompileLog(prog, log);
        printf("Compile Error Log : %s", log);
    }
    // aclrtc流程，获取Device侧二进制内容和大小
    size_t binDataSizeRet;
    ASCENDC_CHECK(aclrtcGetBinDataSize(prog, &binDataSizeRet));
    std::vector<char> deviceELF(binDataSizeRet);
    ASCENDC_CHECK(aclrtcGetBinData(prog, deviceELF.data()));

    ASCENDC_CHECK(aclInit(nullptr));
    ASCENDC_CHECK(aclrtSetDevice(0));
    aclrtStream stream = nullptr;
    ASCENDC_CHECK(aclrtCreateStream(&stream));

    aclrtBinHandle binHandle = nullptr;
    aclrtBinaryLoadOptions loadOption;
    loadOption.numOpt = 1;
    aclrtBinaryLoadOption option;
    option.type = ACL_RT_BINARY_LOAD_OPT_MAGIC;
    option.value.magic = ACL_RT_BINARY_MAGIC_ELF_AICORE;
    loadOption.options = &option;
    ASCENDC_CHECK(aclrtBinaryLoadFromData(deviceELF.data(), binDataSizeRet, &loadOption, &binHandle));

    aclrtFuncHandle funcHandle = nullptr;
    const char* funcName = "hello_world";
    ASCENDC_CHECK(aclrtBinaryGetFunction(binHandle, funcName, &funcHandle));

    // 核函数执行
    uint32_t numBlocks = 8;
    void* kernelArgs[] = {};
    ASCENDC_CHECK(aclrtLaunchKernelWithArgsArray(funcHandle, numBlocks, stream, nullptr, kernelArgs));
    ASCENDC_CHECK(aclrtSynchronizeStream(stream));

    ASCENDC_CHECK(aclrtBinaryUnLoad(binHandle));
    ASCENDC_CHECK(aclrtDestroyStream(stream));
    ASCENDC_CHECK(aclrtResetDevice(0));
    ASCENDC_CHECK(aclFinalize());
    // 编译和运行均已结束，销毁程序
    ASCENDC_CHECK(aclrtcDestroyProg(&prog));
    return 0;
}
