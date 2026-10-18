/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

/* !
 * \file main.cpp
 * \brief Host side code for dynamic library compile sample
 */

#include <iostream>
#include <iterator>
#include <vector>
#include "acl/acl.h"

extern void run_add_custom(
    float* x, float* y, float* z, uint32_t total_length, uint32_t num_blocks, aclrtStream stream);

std::vector<float> add_vectors(std::vector<float>& x, std::vector<float>& y)
{
    if (x.empty() || y.empty()) {
        std::cout << "[ERROR] Empty input tensors." << std::endl;
        return {};
    }

    uint32_t total_length = x.size();
    uint32_t block_length = 2048;
    uint32_t num_blocks = (total_length + block_length - 1) / block_length;

    size_t total_byte_size = total_length * sizeof(float);

    std::cout << "[INFO] Block number is " << num_blocks << "." << std::endl;

    int32_t device_id = 0;
    aclrtStream stream = nullptr;

    uint8_t* x_host = reinterpret_cast<uint8_t*>(x.data());
    uint8_t* y_host = reinterpret_cast<uint8_t*>(y.data());
    uint8_t* z_host = nullptr;
    float* x_device = nullptr;
    float* y_device = nullptr;
    float* z_device = nullptr;

    aclInit(nullptr);
    aclrtSetDevice(device_id);
    aclrtCreateStream(&stream);

    aclrtMallocHost((void**)&z_host, total_byte_size);
    aclrtMalloc((void**)&x_device, total_byte_size, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void**)&y_device, total_byte_size, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void**)&z_device, total_byte_size, ACL_MEM_MALLOC_HUGE_FIRST);

    aclrtMemcpy(x_device, total_byte_size, x_host, total_byte_size, ACL_MEMCPY_HOST_TO_DEVICE);
    aclrtMemcpy(y_device, total_byte_size, y_host, total_byte_size, ACL_MEMCPY_HOST_TO_DEVICE);

    run_add_custom(x_device, y_device, z_device, total_length, num_blocks, stream);

    aclrtSynchronizeStream(stream);
    aclrtMemcpy(z_host, total_byte_size, z_device, total_byte_size, ACL_MEMCPY_DEVICE_TO_HOST);
    std::vector<float> z((float*)z_host, (float*)(z_host + total_byte_size));

    aclrtFree(x_device);
    aclrtFree(y_device);
    aclrtFree(z_device);
    aclrtFreeHost(z_host);

    aclrtDestroyStream(stream);
    aclrtResetDevice(device_id);
    aclFinalize();
    return z;
}

uint32_t verify_result(std::vector<float>& output, std::vector<float>& golden)
{
    auto print_tensor = [](std::vector<float>& tensor, const char* name) {
        constexpr size_t max_print_size = 20;
        std::cout << name << ": ";
        std::copy(
            tensor.begin(), tensor.begin() + std::min(tensor.size(), max_print_size),
            std::ostream_iterator<float>(std::cout, " "));
        if (tensor.size() > max_print_size) {
            std::cout << "...";
        }
        std::cout << std::endl;
    };
    print_tensor(output, "Output");
    print_tensor(golden, "Golden");
    if (std::equal(output.begin(), output.end(), golden.begin())) {
        std::cout << "test pass!" << std::endl;
        return 0;
    } else {
        std::cout << "test failed!" << std::endl;
        return 1;
    }
}

int32_t main(int32_t argc, char* argv[])
{
    constexpr uint32_t total_length = 16384;

    std::vector<float> x(total_length);
    std::vector<float> y(total_length);

    for (uint32_t i = 0; i < total_length; i++) {
        x[i] = i * 0.1f;
        y[i] = i * 0.2f;
    }

    std::vector<float> golden(total_length);
    for (uint32_t i = 0; i < total_length; i++) {
        golden[i] = x[i] + y[i];
    }

    std::vector<float> output = add_vectors(x, y);
    if (output.empty()) {
        return 1;
    }
    return verify_result(output, golden);
}
