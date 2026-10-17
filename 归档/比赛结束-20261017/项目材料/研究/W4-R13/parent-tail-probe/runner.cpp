#include "abi.h"
#include <acl/acl.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

extern "C" void run_kernel_parent(
    uint8_t*, const TensorGroupInfo&, uint8_t*, const TensorGroupInfo&,
    uint8_t*, const TensorGroupInfo&, uint8_t*, const TensorGroupInfo&,
    uint8_t*, const TensorGroupInfo&, int64_t, aclrtStream, float);
extern "C" void record_parent_path(
    void*, void*, void*, void*, void*, void*, uint64_t, uint64_t, uint32_t, aclrtStream);

static void RequireAcl(aclError status, const char* operation)
{
    if (status != ACL_SUCCESS) {
        std::fprintf(stderr, "ACL_ERROR operation=%s status=%d\n", operation, status);
        std::exit(2);
    }
}

static float InputValue(int64_t index, int multiplier, int offset)
{
    return static_cast<float>((index * multiplier + offset) % 997) / 498.0f - 1.0f;
}

int main(int argc, char** argv)
{
    if (argc != 7) {
        std::fprintf(stderr, "usage: %s device rows width pattern runs output_prefix\n", argv[0]);
        return 2;
    }
    const int device = std::atoi(argv[1]);
    const int64_t rows = std::strtoll(argv[2], nullptr, 10);
    const int64_t width = std::strtoll(argv[3], nullptr, 10);
    const int pattern = std::atoi(argv[4]);
    const int runs = std::atoi(argv[5]);
    const std::string prefix = argv[6];
    if (device != 4 || rows < 1 || rows > 128 || width < 8192 || width > 40000 ||
        width % 8 != 0 || pattern < 0 || pattern > 1 || runs < 1 || runs > 3) return 2;
    const size_t count = rows * width, dataBytes = count * sizeof(float);
    const size_t parameterBytes = width * sizeof(float);
    constexpr float epsilon = 1.0e-5f;
    std::vector<float> x(count), residual(count), gamma(width), bias(width), actual(count);
    for (size_t i = 0; i < count; ++i) {
        x[i] = pattern == 0 ? InputValue(i, 37, 11) : 0.5f;
        residual[i] = pattern == 0 ? InputValue(i, 17, 3) : 0.25f;
    }
    for (int64_t j = 0; j < width; ++j) {
        gamma[j] = pattern == 0 ? 0.75f + static_cast<float>((j * 13) % 100) / 200.0f : 1.0f;
        bias[j] = pattern == 0 ? static_cast<float>((j * 7) % 100) / 400.0f - 0.125f : 0.0f;
    }
    std::vector<double> reference(count), roundedReference(count);
    for (int64_t row = 0; row < rows; ++row) {
        double squareSum = 0.0, roundedSquareSum = 0.0;
        for (int64_t j = 0; j < width; ++j) {
            const size_t i = row * width + j;
            const double value = static_cast<double>(x[i]) + residual[i];
            const float roundedValue = x[i] + residual[i];
            squareSum += value * value;
            roundedSquareSum += static_cast<double>(roundedValue) * roundedValue;
        }
        const double invRms = 1.0 / std::sqrt(squareSum / width + epsilon);
        const float roundedInvRms = 1.0f / std::sqrt(static_cast<float>(roundedSquareSum / width) + epsilon);
        for (int64_t j = 0; j < width; ++j) {
            const size_t i = row * width + j;
            reference[i] = (static_cast<double>(x[i]) + residual[i]) * invRms * gamma[j] + bias[j];
            roundedReference[i] = ((x[i] + residual[i]) * roundedInvRms) * gamma[j] + bias[j];
        }
    }

    RequireAcl(aclInit(nullptr), "init");
    RequireAcl(aclrtSetDevice(device), "set_device");
    int64_t cores = 0;
    RequireAcl(aclrtGetDeviceInfo(device, ACL_DEV_ATTR_VECTOR_CORE_NUM, &cores), "vector_cores");
    const uint32_t blocks = static_cast<uint32_t>(std::min(rows, cores));
    aclrtStream stream = nullptr;
    RequireAcl(aclrtCreateStream(&stream), "create_stream");
    void *dx=nullptr, *dr=nullptr, *dg=nullptr, *db=nullptr, *dout=nullptr, *dmeta=nullptr;
    for (void** ptr : {&dx, &dr, &dout}) RequireAcl(aclrtMalloc(ptr, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "alloc_data");
    for (void** ptr : {&dg, &db}) RequireAcl(aclrtMalloc(ptr, parameterBytes, ACL_MEM_MALLOC_HUGE_FIRST), "alloc_parameters");
    RequireAcl(aclrtMalloc(&dmeta, 32, ACL_MEM_MALLOC_HUGE_FIRST), "alloc_metadata");
    RequireAcl(aclrtMemcpy(dx, dataBytes, x.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy_x");
    RequireAcl(aclrtMemcpy(dr, dataBytes, residual.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy_residual");
    RequireAcl(aclrtMemcpy(dg, parameterBytes, gamma.data(), parameterBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy_gamma");
    RequireAcl(aclrtMemcpy(db, parameterBytes, bias.data(), parameterBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy_bias");
    record_parent_path(dx, dr, dg, db, dout, dmeta, rows, width, blocks, stream);
    RequireAcl(aclrtSynchronizeStream(stream), "path_probe");
    int32_t metadata[4]{};
    RequireAcl(aclrtMemcpy(metadata, sizeof(metadata), dmeta, sizeof(metadata), ACL_MEMCPY_DEVICE_TO_HOST), "read_path");
    const int64_t tail = width % metadata[2];
    std::printf("PATH device=%d rows=%lld width=%lld dtype=fp32 cores=%lld blocks=%u wide=%d full_y=%d cache_rows=%d tile=%d tail=%lld\n",
        device, static_cast<long long>(rows), static_cast<long long>(width), static_cast<long long>(cores), blocks,
        metadata[0], metadata[3], metadata[1], metadata[2], static_cast<long long>(tail));
    const int64_t inputShape[] = {rows, width}, parameterShape[] = {width};
    const TensorInfo inputInfo{inputShape, 2, 0}, parameterInfo{parameterShape, 1, 0};
    const TensorGroupInfo inputGroup{&inputInfo, 1}, parameterGroup{&parameterInfo, 1};
    std::ofstream report(prefix + ".tsv");
    report << "run\trows\twidth\tpattern\tcache_rows\ttile\ttail\tregion\telements\tbad\tmax_abs\tmax_abs_rounded\tnonfinite\n";
    bool passed = true;
    for (int run = 0; run < runs; ++run) {
        RequireAcl(aclrtMemset(dout, dataBytes, 0xff, dataBytes), "output_nan");
        run_kernel_parent(static_cast<uint8_t*>(dx),inputGroup,static_cast<uint8_t*>(dr),inputGroup,
            static_cast<uint8_t*>(dg),parameterGroup,static_cast<uint8_t*>(db),parameterGroup,
            static_cast<uint8_t*>(dout),inputGroup,cores,stream,epsilon);
        RequireAcl(aclrtSynchronizeStream(stream), "parent_correctness");
        RequireAcl(aclrtMemcpy(actual.data(), dataBytes, dout, dataBytes, ACL_MEMCPY_DEVICE_TO_HOST), "read_output");
        std::ofstream raw(prefix + "-run" + std::to_string(run) + ".bin", std::ios::binary);
        raw.write(reinterpret_cast<const char*>(actual.data()), dataBytes);
        for (int region = 0; region < 2; ++region) {
            size_t elements = 0, bad = 0, nonfinite = 0;
            double maxAbs = 0.0, maxRounded = 0.0;
            for (size_t i = 0; i < count; ++i) {
                const bool isTail = tail != 0 && static_cast<int64_t>(i % width) >= width - tail;
                if (isTail != (region == 1)) continue;
                ++elements;
                if (!std::isfinite(actual[i])) { ++nonfinite; ++bad; continue; }
                const double error = std::fabs(actual[i] - reference[i]);
                maxAbs = std::max(maxAbs, error);
                maxRounded = std::max(maxRounded, std::fabs(actual[i] - roundedReference[i]));
                if (error > 1.0e-4 + 1.0e-4 * std::fabs(reference[i])) ++bad;
            }
            report.precision(12);
            report << run << '\t' << rows << '\t' << width << '\t' << pattern << '\t' << metadata[1]
                   << '\t' << metadata[2] << '\t' << tail << '\t' << (region ? "tail" : "full")
                   << '\t' << elements << '\t' << bad << '\t' << maxAbs << '\t' << maxRounded << '\t' << nonfinite << '\n';
            std::printf("CORRECTNESS run=%d pattern=%d region=%s elements=%zu bad=%zu max_abs=%.9g max_abs_rounded=%.9g nonfinite=%zu\n",
                run,pattern,region ? "tail" : "full",elements,bad,maxAbs,maxRounded,nonfinite);
            passed = passed && bad == 0;
        }
        if (!raw.good()) return 2;
    }
    for (void* ptr : {dx, dr, dg, db, dout, dmeta}) RequireAcl(aclrtFree(ptr), "free");
    RequireAcl(aclrtDestroyStream(stream), "destroy_stream");
    RequireAcl(aclFinalize(), "finalize");
    return report.good() ? (passed ? 0 : 3) : 2;
}
