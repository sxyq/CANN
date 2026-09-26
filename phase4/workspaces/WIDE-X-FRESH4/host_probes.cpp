#define FRESH4_HOST_PROBE
#include "wide_x_fresh4.asc"

#include <algorithm>
#include <cstdio>
#include <vector>

static bool ProbeWidth(int64_t width)
{
    constexpr int64_t rows = 2;
    constexpr float epsilon = 1.0e-5f;
    const int64_t inputShape[] = {rows, width};
    const int64_t vectorShape[] = {width};
    const TensorInfo inputInfo{inputShape, 2, 0};
    const TensorInfo vectorInfo{vectorShape, 1, 0};
    const TensorGroupInfo inputGroup{&inputInfo, 1};
    const TensorGroupInfo vectorGroup{&vectorInfo, 1};
    std::vector<float> x(rows * width), residual(rows * width), gamma(width), bias(width), output(rows * width, 0.0f);
    std::vector<float> expected(rows * width);
    for (int64_t j = 0; j < width; ++j) {
        gamma[j] = 0.75f + static_cast<float>(j % 7) * 0.03125f;
        bias[j] = static_cast<float>(j % 11 - 5) * 0.002f;
    }
    for (int64_t i = 0; i < rows * width; ++i) {
        x[i] = static_cast<float>(i % 37 - 18) * 0.015f;
        residual[i] = static_cast<float>(i % 19 - 9) * 0.009f;
    }
    const TensorInfo outInfo{inputShape, 2, 0};
    const TensorGroupInfo outGroup{&outInfo, 1};
    run_kernel(x.data(), inputGroup, residual.data(), inputGroup,
               gamma.data(), vectorGroup, bias.data(), vectorGroup,
               output.data(), outGroup, 40, nullptr, epsilon);
    for (int64_t r = 0; r < rows; ++r) {
        double sum = 0.0;
        for (int64_t j = 0; j < width; ++j) {
            const float u = x[r * width + j] + residual[r * width + j];
            sum += static_cast<double>(u) * u;
        }
        const float invRms = 1.0f / std::sqrt(static_cast<float>(sum / width) + epsilon);
        for (int64_t j = 0; j < width; ++j) {
            const float u = x[r * width + j] + residual[r * width + j];
            expected[r * width + j] = u * invRms * gamma[j] + bias[j];
        }
    }
    for (size_t i = 0; i < output.size(); ++i) {
        if (std::abs(output[i] - expected[i]) > 2.0e-6f) {
            std::fprintf(stderr, "width=%lld mismatch at %zu: got %.9g expected %.9g\n",
                         static_cast<long long>(width), i, output[i], expected[i]);
            return false;
        }
    }
    std::printf("host probe passed: D=%lld R=%lld\n", static_cast<long long>(width), static_cast<long long>(rows));
    return true;
}

int main()
{
    return ProbeWidth(16384) && ProbeWidth(32768) ? 0 : 1;
}
