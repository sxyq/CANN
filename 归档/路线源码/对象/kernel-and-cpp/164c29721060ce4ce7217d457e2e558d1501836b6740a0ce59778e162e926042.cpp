#include <acl/acl.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include "kernel_entry_abi.hpp"

#ifndef KERNEL_SOURCE_SHA256
#define KERNEL_SOURCE_SHA256 "unknown"
#endif
#ifndef RUNNER_EXECUTABLE_PATH
#define RUNNER_EXECUTABLE_PATH "route_runner"
#endif

#ifndef V034_RUNNER_ENTRY_DEFINED
extern "C" void run_kernel(
    KernelGmAddr x, const TensorGroupInfo& info_x,
    KernelGmAddr residual, const TensorGroupInfo& info_residual,
    KernelGmAddr gamma, const TensorGroupInfo& info_gamma,
    KernelGmAddr bias, const TensorGroupInfo& info_bias,
    KernelGmAddr output, const TensorGroupInfo& info_output,
    std::int64_t availableCoreNum, aclrtStream stream, float epsilon);
#endif

namespace {

void CheckAcl(aclError status, const char* operation)
{
    if (status != ACL_SUCCESS) {
        throw std::runtime_error(std::string(operation) + " failed: " + std::to_string(status));
    }
}

struct Options {
    uint32_t device = 1;
    int32_t dtype = -1;
    float epsilon = 1.0e-5f;
    int32_t warmup = 0;
    int32_t repeat = 1;
    std::vector<int64_t> shape;
    std::string xPath;
    std::string residualPath;
    std::string gammaPath;
    std::string biasPath;
    std::string outputPath;
};

Options ParseOptions(int argc, char** argv)
{
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string key = argv[i];
        if (i + 1 >= argc) {
            throw std::runtime_error("missing value for " + key);
        }
        const std::string value = argv[++i];
        if (key == "--device") {
            options.device = static_cast<uint32_t>(std::stoul(value));
        } else if (key == "--dtype") {
            options.dtype = std::stoi(value);
        } else if (key == "--epsilon") {
            options.epsilon = std::stof(value);
        } else if (key == "--warmup") {
            options.warmup = std::stoi(value);
        } else if (key == "--repeat") {
            options.repeat = std::stoi(value);
        } else if (key == "--shape") {
            size_t begin = 0;
            while (begin < value.size()) {
                const size_t end = value.find(',', begin);
                options.shape.push_back(std::stoll(value.substr(begin, end - begin)));
                if (end == std::string::npos) break;
                begin = end + 1;
            }
        } else if (key == "--x") {
            options.xPath = value;
        } else if (key == "--residual") {
            options.residualPath = value;
        } else if (key == "--gamma") {
            options.gammaPath = value;
        } else if (key == "--bias") {
            options.biasPath = value;
        } else if (key == "--output") {
            options.outputPath = value;
        } else {
            throw std::runtime_error("unknown option: " + key);
        }
    }
    if (options.dtype < 0 || options.dtype > 2 || options.shape.empty() ||
        options.warmup < 0 || options.repeat < 1 || options.epsilon < 0.0f ||
        options.xPath.empty() || options.residualPath.empty() || options.gammaPath.empty() ||
        options.biasPath.empty() || options.outputPath.empty()) {
        throw std::runtime_error("invalid or incomplete arguments");
    }
    for (const int64_t extent : options.shape) {
        if (extent <= 0) throw std::runtime_error("all dimensions must be positive");
    }
    return options;
}

size_t TypeBytes(int32_t dtype)
{
    return dtype == 0 ? sizeof(float) : sizeof(uint16_t);
}

size_t ElementCount(const std::vector<int64_t>& shape)
{
    size_t count = 1;
    for (const int64_t extent : shape) {
        if (count > std::numeric_limits<size_t>::max() / static_cast<size_t>(extent)) {
            throw std::runtime_error("tensor element count overflow");
        }
        count *= static_cast<size_t>(extent);
    }
    return count;
}

std::vector<uint8_t> ReadExact(const std::string& path, size_t bytes)
{
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input || static_cast<size_t>(input.tellg()) != bytes) {
        throw std::runtime_error("input file missing or has wrong size: " + path);
    }
    std::vector<uint8_t> data(bytes);
    input.seekg(0);
    input.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(bytes));
    if (!input) throw std::runtime_error("failed reading input: " + path);
    return data;
}

struct DeviceBuffer {
    uint8_t* data = nullptr;
    explicit DeviceBuffer(size_t bytes)
    {
        void* allocation = nullptr;
        CheckAcl(aclrtMalloc(&allocation, bytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc");
        data = static_cast<uint8_t*>(allocation);
    }
    ~DeviceBuffer()
    {
        if (data != nullptr) aclrtFree(data);
    }
    void Free()
    {
        if (data != nullptr) {
            CheckAcl(aclrtFree(data), "aclrtFree");
            data = nullptr;
        }
    }
    DeviceBuffer(const DeviceBuffer&) = delete;
    DeviceBuffer& operator=(const DeviceBuffer&) = delete;
};

void CopyToDevice(DeviceBuffer& destination, const std::vector<uint8_t>& source)
{
    CheckAcl(aclrtMemcpy(destination.data, source.size(), source.data(), source.size(),
                         ACL_MEMCPY_HOST_TO_DEVICE), "aclrtMemcpy H2D");
}

void Run(const Options& options)
{
    const size_t width = static_cast<size_t>(options.shape.back());
    const size_t elements = ElementCount(options.shape);
    const uint64_t rowCount = static_cast<uint64_t>(elements / width);
    const uint64_t rowWidth = static_cast<uint64_t>(width);
    const size_t typeBytes = TypeBytes(options.dtype);
    const size_t tensorBytes = elements * typeBytes;
    const size_t parameterBytes = width * typeBytes;
    const std::vector<uint8_t> xHost = ReadExact(options.xPath, tensorBytes);
    const std::vector<uint8_t> residualHost = ReadExact(options.residualPath, tensorBytes);
    const std::vector<uint8_t> gammaHost = ReadExact(options.gammaPath, parameterBytes);
    const std::vector<uint8_t> biasHost = ReadExact(options.biasPath, parameterBytes);
    CheckAcl(aclInit(nullptr), "aclInit");
    aclrtStream stream = nullptr;
    aclrtEvent start = nullptr;
    aclrtEvent end = nullptr;
    bool aclInitialized = true;
    try {
        CheckAcl(aclrtSetDevice(options.device), "aclrtSetDevice");
        CheckAcl(aclrtCreateStream(&stream), "aclrtCreateStream");

        int64_t coreCount = 0;
        CheckAcl(aclrtGetDeviceInfo(options.device, ACL_DEV_ATTR_VECTOR_CORE_NUM, &coreCount),
                 "aclrtGetDeviceInfo(vector core count)");
        if (coreCount <= 0) throw std::runtime_error("device reported no vector cores");
        const std::int64_t tensorRank = static_cast<std::int64_t>(options.shape.size());
        const std::int64_t parameterShape[] = {static_cast<std::int64_t>(width)};
        const TensorInfo xInfo{options.shape.data(), tensorRank, options.dtype};
        const TensorInfo residualInfo{options.shape.data(), tensorRank, options.dtype};
        const TensorInfo gammaInfo{parameterShape, 1, options.dtype};
        const TensorInfo biasInfo{parameterShape, 1, options.dtype};
        const TensorInfo outputInfo{options.shape.data(), tensorRank, options.dtype};
        const TensorGroupInfo xGroup{&xInfo, 1};
        const TensorGroupInfo residualGroup{&residualInfo, 1};
        const TensorGroupInfo gammaGroup{&gammaInfo, 1};
        const TensorGroupInfo biasGroup{&biasInfo, 1};
        const TensorGroupInfo outputGroup{&outputInfo, 1};

        DeviceBuffer xDevice(tensorBytes);
        DeviceBuffer residualDevice(tensorBytes);
        DeviceBuffer gammaDevice(parameterBytes);
        DeviceBuffer biasDevice(parameterBytes);
        DeviceBuffer outputDevice(tensorBytes);
        CopyToDevice(xDevice, xHost);
        CopyToDevice(residualDevice, residualHost);
        CopyToDevice(gammaDevice, gammaHost);
        CopyToDevice(biasDevice, biasHost);
        std::vector<uint8_t> outputHost(tensorBytes, 0xff);
        CheckAcl(aclrtMemcpy(outputDevice.data, tensorBytes, outputHost.data(), tensorBytes,
                             ACL_MEMCPY_HOST_TO_DEVICE), "aclrtMemcpy output sentinel");

        auto invoke = [&]() {
            run_kernel(xDevice.data, xGroup, residualDevice.data, residualGroup,
                       gammaDevice.data, gammaGroup, biasDevice.data, biasGroup,
                       outputDevice.data, outputGroup, coreCount, stream, options.epsilon);
        };
        for (int32_t i = 0; i < options.warmup; ++i) invoke();
        CheckAcl(aclrtSynchronizeStream(stream), "aclrtSynchronizeStream(warmup)");
        CheckAcl(aclrtCreateEvent(&start), "aclrtCreateEvent(start)");
        CheckAcl(aclrtCreateEvent(&end), "aclrtCreateEvent(end)");

        std::vector<float> samples;
        samples.reserve(static_cast<size_t>(options.repeat));
        for (int32_t i = 0; i < options.repeat; ++i) {
            CheckAcl(aclrtRecordEvent(start, stream), "aclrtRecordEvent(start)");
            invoke();
            CheckAcl(aclrtRecordEvent(end, stream), "aclrtRecordEvent(end)");
            CheckAcl(aclrtSynchronizeEvent(end), "aclrtSynchronizeEvent(end)");
            float elapsedMs = 0.0f;
            CheckAcl(aclrtEventElapsedTime(&elapsedMs, start, end), "aclrtEventElapsedTime");
            samples.push_back(elapsedMs * 1000.0f);
        }

        CheckAcl(aclrtMemcpy(outputHost.data(), tensorBytes, outputDevice.data, tensorBytes,
                             ACL_MEMCPY_DEVICE_TO_HOST), "aclrtMemcpy D2H");
        std::ofstream output(options.outputPath, std::ios::binary | std::ios::trunc);
        output.write(reinterpret_cast<const char*>(outputHost.data()),
                     static_cast<std::streamsize>(outputHost.size()));
        if (!output) throw std::runtime_error("failed writing output: " + options.outputPath);

        std::cout << "KERNEL_SOURCE_SHA256=" << KERNEL_SOURCE_SHA256 << '\n';
        std::cout << "RUNNER_EXECUTABLE_PATH=" << RUNNER_EXECUTABLE_PATH << '\n';
        std::cout << "DEVICE=" << options.device << '\n';
        std::cout << "VECTOR_CORE_COUNT=" << coreCount << '\n';
        for (const float sample : samples) std::cout << "LATENCY_US=" << sample << '\n';

        outputDevice.Free();
        biasDevice.Free();
        gammaDevice.Free();
        residualDevice.Free();
        xDevice.Free();
        CheckAcl(aclrtDestroyEvent(end), "aclrtDestroyEvent(end)");
        end = nullptr;
        CheckAcl(aclrtDestroyEvent(start), "aclrtDestroyEvent(start)");
        start = nullptr;
        CheckAcl(aclrtDestroyStream(stream), "aclrtDestroyStream");
        stream = nullptr;
        CheckAcl(aclFinalize(), "aclFinalize");
        aclInitialized = false;
    } catch (...) {
        if (end != nullptr) aclrtDestroyEvent(end);
        if (start != nullptr) aclrtDestroyEvent(start);
        if (stream != nullptr) aclrtDestroyStream(stream);
        if (aclInitialized) aclFinalize();
        throw;
    }
}

}  // namespace

int main(int argc, char** argv)
{
    try {
        Run(ParseOptions(argc, argv));
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "RUNNER_ERROR=" << error.what() << '\n';
        return 2;
    }
}
