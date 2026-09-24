#define FRESH4_HOST_PROBE
#ifndef FRESH4_PROBE_SOURCE
#define FRESH4_PROBE_SOURCE "wide_x_fresh4.asc"
#endif
#include FRESH4_PROBE_SOURCE

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace {

constexpr int64_t kRows = 2;
constexpr float kEpsilon = 1.0e-5f;
constexpr int kWarmupRuns = 2;
constexpr int kMeasuredRuns = 8;

bool RunProbe(const char* label, int64_t width)
{
    const int64_t inputElements = kRows * width;
    const int64_t vectorElements = width;
    const int64_t inputShape[] = {kRows, width};
    const int64_t vectorShape[] = {width};
    const TensorInfo inputInfo{inputShape, 2, 0};
    const TensorInfo vectorInfo{vectorShape, 1, 0};
    const TensorGroupInfo inputGroup{&inputInfo, 1};
    const TensorGroupInfo vectorGroup{&vectorInfo, 1};
    std::vector<float> x(inputElements), residual(inputElements);
    std::vector<float> gamma(vectorElements), bias(vectorElements);
    std::vector<float> output(inputElements, 0.0f);
    std::vector<double> latencyNs;
    latencyNs.reserve(kMeasuredRuns);

    for (int64_t j = 0; j < width; ++j) {
        gamma[j] = 0.75f + static_cast<float>(j % 7) * 0.03125f;
        bias[j] = static_cast<float>(j % 11 - 5) * 0.002f;
    }
    for (int64_t i = 0; i < inputElements; ++i) {
        x[i] = static_cast<float>(i % 37 - 18) * 0.015f;
        residual[i] = static_cast<float>(i % 19 - 9) * 0.009f;
    }
    const TensorInfo outputInfo{inputShape, 2, 0};
    const TensorGroupInfo outputGroup{&outputInfo, 1};

    for (int i = 0; i < kWarmupRuns; ++i) {
        run_kernel(x.data(), inputGroup, residual.data(), inputGroup,
                   gamma.data(), vectorGroup, bias.data(), vectorGroup,
                   output.data(), outputGroup, 40, nullptr, kEpsilon);
    }
    for (int i = 0; i < kMeasuredRuns; ++i) {
        const auto start = std::chrono::steady_clock::now();
        run_kernel(x.data(), inputGroup, residual.data(), inputGroup,
                   gamma.data(), vectorGroup, bias.data(), vectorGroup,
                   output.data(), outputGroup, 40, nullptr, kEpsilon);
        const auto finish = std::chrono::steady_clock::now();
        latencyNs.push_back(static_cast<double>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count()));
    }

    float maxError = 0.0f;
    for (int64_t r = 0; r < kRows; ++r) {
        double squareSum = 0.0;
        for (int64_t j = 0; j < width; ++j) {
            const float u = x[r * width + j] + residual[r * width + j];
            squareSum += static_cast<double>(u) * u;
        }
        const float invRms = 1.0f / std::sqrt(static_cast<float>(squareSum / width) + kEpsilon);
        for (int64_t j = 0; j < width; ++j) {
            const float u = x[r * width + j] + residual[r * width + j];
            const float expected = u * invRms * gamma[j] + bias[j];
            maxError = std::max(maxError, std::abs(output[r * width + j] - expected));
        }
    }

    std::printf("label=%s width=%lld correctness=%s max_abs_error=%.9g latency_ns=",
                label, static_cast<long long>(width), maxError <= 2.0e-6f ? "PASS" : "FAIL", maxError);
    for (size_t i = 0; i < latencyNs.size(); ++i) {
        if (i != 0) std::printf(",");
        std::printf("%.0f", latencyNs[i]);
    }
    std::printf("\n");
    return maxError <= 2.0e-6f;
}

}  // namespace

int main(int argc, char** argv)
{
    if (argc != 3) {
        std::fprintf(stderr, "usage: %s LABEL WIDTH\n", argv[0]);
        return 2;
    }
    const int64_t width = std::strtoll(argv[2], nullptr, 10);
    return RunProbe(argv[1], width) ? 0 : 1;
}
