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
 * \brief Runs and verifies the SIMT gather sample built as a static library.
 */

#include <iostream>
#include <iterator>
#include <vector>
#include "acl/acl.h"

extern void run_gather_custom(
    float* input, int32_t* index, float* output, uint32_t input_total_length, uint32_t index_total_length,
    uint32_t blocks_per_grid, uint32_t threads_per_block, uint32_t dyn_ubuf_size, aclrtStream stream);

std::vector<float> gather_1d(std::vector<float>& input, std::vector<int32_t>& index)
{
    if (input.empty() || index.empty()) {
        std::cout << "[ERROR] Empty input tensors." << std::endl;
        return {};
    }

    uint32_t input_total_length = input.size();
    uint32_t index_total_length = index.size();

    size_t input_total_byte_size = input_total_length * sizeof(float);
    size_t index_total_byte_size = index_total_length * sizeof(int32_t);
    size_t output_total_byte_size = index_total_length * sizeof(float);

    uint32_t threads_per_block = 2048;
    uint32_t blocks_per_grid = (index_total_length + threads_per_block - 1) / threads_per_block;

    std::cout << "[INFO] Block number is " << blocks_per_grid << "." << std::endl;
    std::cout << "[INFO] Thread number in a block is " << threads_per_block << "." << std::endl;

    int32_t device_id = 0;
    aclrtStream stream = nullptr;

    uint8_t* input_host = reinterpret_cast<uint8_t*>(input.data());
    uint8_t* index_host = reinterpret_cast<uint8_t*>(index.data());
    uint8_t* output_host = nullptr;
    float* input_device = nullptr;
    int32_t* index_device = nullptr;
    float* output_device = nullptr;

    aclInit(nullptr);
    aclrtSetDevice(device_id);
    aclrtCreateStream(&stream);

    aclrtMallocHost((void**)(&output_host), output_total_byte_size);
    aclrtMalloc((void**)&input_device, input_total_byte_size, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void**)&index_device, index_total_byte_size, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void**)&output_device, output_total_byte_size, ACL_MEM_MALLOC_HUGE_FIRST);

    aclrtMemcpy(input_device, input_total_byte_size, input_host, input_total_byte_size, ACL_MEMCPY_HOST_TO_DEVICE);
    aclrtMemcpy(index_device, index_total_byte_size, index_host, index_total_byte_size, ACL_MEMCPY_HOST_TO_DEVICE);

    uint32_t dyn_ubuf_size = 0;
    run_gather_custom(
        input_device, index_device, output_device, input_total_length, index_total_length, blocks_per_grid,
        threads_per_block, dyn_ubuf_size, stream);

    aclrtSynchronizeStream(stream);
    aclrtMemcpy(output_host, output_total_byte_size, output_device, output_total_byte_size, ACL_MEMCPY_DEVICE_TO_HOST);
    std::vector<float> output((float*)output_host, (float*)(output_host + output_total_byte_size));

    aclrtFree(input_device);
    aclrtFree(index_device);
    aclrtFree(output_device);
    aclrtFreeHost(output_host);

    const char* err = aclGetRecentErrMsg();
    if (err != nullptr) {
        fprintf(stderr, "%s\n", err);
    }

    aclrtDestroyStream(stream);
    aclrtResetDevice(device_id);
    aclFinalize();
    return output;
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
        std::cout << "[Success] Case accuracy is verification passed." << std::endl;
        return 0;
    } else {
        std::cout << "[Failed] Case accuracy is verification failed!" << std::endl;
        return 1;
    }
    return 0;
}

int32_t main(int32_t argc, char* argv[])
{
    constexpr uint32_t input_total_length = 100000;
    constexpr uint32_t index_total_length = 12288;

    std::vector<float> input(input_total_length);
    for (uint32_t i = 0; i < input_total_length; i++) {
        input[i] = i * 1.2f;
    }

    std::vector<int32_t> index(index_total_length);
    for (uint32_t i = 0; i < index_total_length; i++) {
        index[i] = static_cast<int32_t>((i * 97 + 13) % input_total_length);
    }

    std::vector<float> golden(index_total_length);
    for (uint32_t i = 0; i < index_total_length; i++) {
        golden[i] = input[index[i]];
    }

    std::vector<float> output = gather_1d(input, index);
    if (output.empty()) {
        return 1;
    }
    return verify_result(output, golden);
}
