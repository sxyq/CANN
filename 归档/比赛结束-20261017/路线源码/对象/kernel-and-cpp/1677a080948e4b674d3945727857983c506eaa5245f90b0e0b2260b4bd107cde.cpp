#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <vector>

#ifndef UB_DIAG_SOURCE_LABEL
#define UB_DIAG_SOURCE_LABEL "UNDEFINED_SOURCE"
#endif
#ifndef UB_DIAG_SOURCE_PATH
#define UB_DIAG_SOURCE_PATH "UNDEFINED_PATH"
#endif
#ifndef UB_DIAG_SOURCE_SHA
#define UB_DIAG_SOURCE_SHA "UNDEFINED_SHA"
#endif

namespace ub_diag_2 {

constexpr float kEpsilon = 1.0e-5f;
constexpr int kFp32 = 0;
constexpr int kFp16 = 1;
constexpr int kBf16 = 2;
constexpr int kTileElems = 4096;

struct CaseSpec {
    const char* name;
    int rows;
    int width;
    int dtype;
};

struct Observation {
    bool launch_ok = false;
    aclError sync_status = ACL_SUCCESS;
    bool copy_ok = false;
    std::vector<unsigned char> actual_bytes;
    std::vector<float> reference;
    std::vector<float> reference_inv_rms;
    size_t failures = 0;
    size_t actual_nan = 0;
    size_t actual_inf = 0;
    size_t reference_nan = 0;
    size_t reference_inf = 0;
    size_t first_mismatch = std::numeric_limits<size_t>::max();
    float first_actual = 0.0f;
    float first_reference = 0.0f;
    float first_abs = 0.0f;
    float first_rel = 0.0f;
    float max_abs = 0.0f;
    float max_rel = 0.0f;
};

uint16_t ToHalf(float value)
{
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    const uint32_t sign = (bits >> 16) & 0x8000u;
    const uint32_t exponent = (bits >> 23) & 0xffu;
    uint32_t mantissa = bits & 0x7fffffu;
    if (exponent == 0xffu) {
        return static_cast<uint16_t>(sign | 0x7c00u | (mantissa ? 0x200u : 0));
    }
    int exponent16 = static_cast<int>(exponent) - 112;
    if (exponent16 >= 31) {
        return static_cast<uint16_t>(sign | 0x7c00u);
    }
    if (exponent16 <= 0) {
        if (exponent16 < -10) {
            return static_cast<uint16_t>(sign);
        }
        mantissa |= 0x800000u;
        const int shift = 14 - exponent16;
        const uint32_t round = (1u << (shift - 1)) - 1u + ((mantissa >> shift) & 1u);
        return static_cast<uint16_t>(sign | ((mantissa + round) >> shift));
    }
    mantissa += 0xfffu + ((mantissa >> 13) & 1u);
    if (mantissa & 0x800000u) {
        mantissa = 0;
        if (++exponent16 >= 31) {
            return static_cast<uint16_t>(sign | 0x7c00u);
        }
    }
    return static_cast<uint16_t>(sign | (static_cast<uint32_t>(exponent16) << 10) |
                                 (mantissa >> 13));
}

float FromHalf(uint16_t value)
{
    const float sign = (value & 0x8000u) ? -1.0f : 1.0f;
    const uint32_t exponent = (value >> 10) & 0x1fu;
    const uint32_t mantissa = value & 0x3ffu;
    if (!exponent) {
        return sign * std::ldexp(static_cast<float>(mantissa), -24);
    }
    if (exponent == 0x1fu) {
        return mantissa ? std::numeric_limits<float>::quiet_NaN() : sign * INFINITY;
    }
    return sign * std::ldexp(1.0f + static_cast<float>(mantissa) / 1024.0f,
                             static_cast<int>(exponent) - 15);
}

uint16_t ToBf16(float value)
{
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    bits += 0x7fffu + ((bits >> 16) & 1u);
    return static_cast<uint16_t>(bits >> 16);
}

float FromBf16(uint16_t value)
{
    const uint32_t bits = static_cast<uint32_t>(value) << 16;
    float result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

size_t ElementBytes(int dtype)
{
    return dtype == kFp32 ? sizeof(float) : sizeof(uint16_t);
}

void StoreValue(std::vector<unsigned char>& bytes, size_t index, int dtype, float value)
{
    if (dtype == kFp32) {
        std::memcpy(bytes.data() + index * sizeof(float), &value, sizeof(value));
        return;
    }
    const uint16_t encoded = dtype == kFp16 ? ToHalf(value) : ToBf16(value);
    std::memcpy(bytes.data() + index * sizeof(encoded), &encoded, sizeof(encoded));
}

float LoadValue(const std::vector<unsigned char>& bytes, size_t index, int dtype)
{
    if (dtype == kFp32) {
        float value = 0.0f;
        std::memcpy(&value, bytes.data() + index * sizeof(value), sizeof(value));
        return value;
    }
    uint16_t encoded = 0;
    std::memcpy(&encoded, bytes.data() + index * sizeof(encoded), sizeof(encoded));
    return dtype == kFp16 ? FromHalf(encoded) : FromBf16(encoded);
}

float RoundDtype(float value, int dtype)
{
    if (dtype == kFp32) {
        return value;
    }
    return dtype == kFp16 ? FromHalf(ToHalf(value)) : FromBf16(ToBf16(value));
}

bool CheckAcl(aclError status, const char* operation)
{
    if (status == ACL_SUCCESS) {
        return true;
    }
    std::fprintf(stderr, "DIAG_ACL_FAIL source=%s operation=%s status=%d\n",
                 UB_DIAG_SOURCE_LABEL, operation, status);
    return false;
}

void BuildInputs(const CaseSpec& spec,
                 std::vector<unsigned char>& x,
                 std::vector<unsigned char>& residual,
                 std::vector<unsigned char>& gamma,
                 std::vector<unsigned char>& bias)
{
    const size_t count = static_cast<size_t>(spec.rows) * spec.width;
    const size_t elementBytes = ElementBytes(spec.dtype);
    x.resize(count * elementBytes);
    residual.resize(count * elementBytes);
    gamma.resize(static_cast<size_t>(spec.width) * elementBytes);
    bias.resize(static_cast<size_t>(spec.width) * elementBytes);
    for (size_t i = 0; i < count; ++i) {
        const float xValue = 0.19f * std::sin(static_cast<float>((i * 17) % 997) * 0.013f);
        const float residualValue =
            0.11f * std::cos(static_cast<float>((i * 29) % 991) * 0.017f);
        StoreValue(x, i, spec.dtype, xValue);
        StoreValue(residual, i, spec.dtype, residualValue);
    }
    for (int col = 0; col < spec.width; ++col) {
        StoreValue(gamma, static_cast<size_t>(col), spec.dtype,
                   0.85f + static_cast<float>(col % 23) * 0.002f);
        StoreValue(bias, static_cast<size_t>(col), spec.dtype,
                   -0.025f + static_cast<float>(col % 19) * 0.001f);
    }
}

void BuildReference(const CaseSpec& spec,
                    const std::vector<unsigned char>& x,
                    const std::vector<unsigned char>& residual,
                    const std::vector<unsigned char>& gamma,
                    const std::vector<unsigned char>& bias,
                    std::vector<float>& reference,
                    std::vector<float>& inv_rms)
{
    const size_t count = static_cast<size_t>(spec.rows) * spec.width;
    reference.resize(count);
    inv_rms.resize(static_cast<size_t>(spec.rows));
    for (int row = 0; row < spec.rows; ++row) {
        float square_sum = 0.0f;
        for (int col = 0; col < spec.width; ++col) {
            const size_t index = static_cast<size_t>(row) * spec.width + col;
            float y = LoadValue(x, index, spec.dtype) + LoadValue(residual, index, spec.dtype);
            if (spec.dtype == kFp16) {
                y = RoundDtype(y, spec.dtype);
            }
            square_sum += y * y;
        }
        const float inv = 1.0f / std::sqrt(square_sum / static_cast<float>(spec.width) + kEpsilon);
        inv_rms[static_cast<size_t>(row)] = inv;
        for (int col = 0; col < spec.width; ++col) {
            const size_t index = static_cast<size_t>(row) * spec.width + col;
            const float y = [&]() {
                float value = LoadValue(x, index, spec.dtype) + LoadValue(residual, index, spec.dtype);
                return spec.dtype == kFp16 ? RoundDtype(value, spec.dtype) : value;
            }();
            float expected = y * inv;
            if (spec.dtype == kFp16) {
                expected = RoundDtype(expected, spec.dtype);
                expected = RoundDtype(expected * LoadValue(gamma, static_cast<size_t>(col), spec.dtype),
                                      spec.dtype);
                expected = RoundDtype(expected + LoadValue(bias, static_cast<size_t>(col), spec.dtype),
                                      spec.dtype);
            } else {
                expected = y * inv * LoadValue(gamma, static_cast<size_t>(col), spec.dtype) +
                           LoadValue(bias, static_cast<size_t>(col), spec.dtype);
                if (spec.dtype == kBf16) {
                    expected = RoundDtype(expected, spec.dtype);
                }
            }
            reference[index] = expected;
        }
    }
}

bool IsMismatch(float actual, float expected, float atol, float rtol)
{
    if (!std::isfinite(actual) || !std::isfinite(expected)) {
        return true;
    }
    const float abs_error = std::fabs(actual - expected);
    return abs_error > atol + rtol * std::fabs(expected);
}

bool RunCase(const CaseSpec& spec, int device, int blocks, int run, Observation& observation)
{
    const size_t count = static_cast<size_t>(spec.rows) * spec.width;
    const size_t elementBytes = ElementBytes(spec.dtype);
    const size_t dataBytes = count * elementBytes;
    const size_t parameterBytes = static_cast<size_t>(spec.width) * elementBytes;
    std::vector<unsigned char> x;
    std::vector<unsigned char> residual;
    std::vector<unsigned char> gamma;
    std::vector<unsigned char> bias;
    BuildInputs(spec, x, residual, gamma, bias);
    BuildReference(spec, x, residual, gamma, bias, observation.reference,
                   observation.reference_inv_rms);

    void* dx = nullptr;
    void* dr = nullptr;
    void* dg = nullptr;
    void* db = nullptr;
    void* output = nullptr;
    aclrtStream stream = nullptr;
    bool ok = true;
    ok = ok && CheckAcl(aclrtMalloc(&dx, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc_x");
    ok = ok && CheckAcl(aclrtMalloc(&dr, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc_residual");
    ok = ok && CheckAcl(aclrtMalloc(&dg, parameterBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc_gamma");
    ok = ok && CheckAcl(aclrtMalloc(&db, parameterBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc_bias");
    ok = ok && CheckAcl(aclrtMalloc(&output, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc_output");
    if (ok) ok = CheckAcl(aclrtMemcpy(dx, dataBytes, x.data(), dataBytes,
                                      ACL_MEMCPY_HOST_TO_DEVICE), "copy_x");
    if (ok) ok = CheckAcl(aclrtMemcpy(dr, dataBytes, residual.data(), dataBytes,
                                      ACL_MEMCPY_HOST_TO_DEVICE), "copy_residual");
    if (ok) ok = CheckAcl(aclrtMemcpy(dg, parameterBytes, gamma.data(), parameterBytes,
                                      ACL_MEMCPY_HOST_TO_DEVICE), "copy_gamma");
    if (ok) ok = CheckAcl(aclrtMemcpy(db, parameterBytes, bias.data(), parameterBytes,
                                      ACL_MEMCPY_HOST_TO_DEVICE), "copy_bias");
    if (ok) ok = CheckAcl(aclrtCreateStream(&stream), "create_stream");

    if (ok) {
        const int64_t dataShape[] = {spec.rows, spec.width};
        const int64_t parameterShape[] = {spec.width};
        TensorInfo dataInfo{dataShape, 2, spec.dtype};
        TensorInfo parameterInfo{parameterShape, 1, spec.dtype};
        TensorGroupInfo dataGroup{&dataInfo, 1};
        TensorGroupInfo parameterGroup{&parameterInfo, 1};
        run_kernel((GM_ADDR)dx, dataGroup, (GM_ADDR)dr, dataGroup,
                   (GM_ADDR)dg, parameterGroup, (GM_ADDR)db, parameterGroup,
                   (GM_ADDR)output, dataGroup, blocks, stream, kEpsilon);
        observation.launch_ok = true;
        observation.sync_status = aclrtSynchronizeStream(stream);
        if (observation.sync_status != ACL_SUCCESS) {
            std::fprintf(stderr,
                         "DIAG_SYNC_FAIL source=%s run=%d case=%s status=%d\n",
                         UB_DIAG_SOURCE_LABEL, run, spec.name, observation.sync_status);
        }
        observation.actual_bytes.resize(dataBytes);
        observation.copy_ok = CheckAcl(aclrtMemcpy(observation.actual_bytes.data(), dataBytes,
                                                   output, dataBytes,
                                                   ACL_MEMCPY_DEVICE_TO_HOST), "copy_output");
    }

    if (stream != nullptr) {
        CheckAcl(aclrtDestroyStream(stream), "destroy_stream");
    }
    if (output != nullptr) CheckAcl(aclrtFree(output), "free_output");
    if (db != nullptr) CheckAcl(aclrtFree(db), "free_bias");
    if (dg != nullptr) CheckAcl(aclrtFree(dg), "free_gamma");
    if (dr != nullptr) CheckAcl(aclrtFree(dr), "free_residual");
    if (dx != nullptr) CheckAcl(aclrtFree(dx), "free_x");

    if (!ok || !observation.launch_ok || !observation.copy_ok) {
        return false;
    }

    const float atol = spec.dtype == kFp16 ? 0.004f : (spec.dtype == kBf16 ? 0.02f : 1.0e-4f);
    const float rtol = atol;
    for (size_t index = 0; index < count; ++index) {
        const float actual = LoadValue(observation.actual_bytes, index, spec.dtype);
        const float expected = observation.reference[index];
        if (std::isnan(actual)) ++observation.actual_nan;
        if (std::isinf(actual)) ++observation.actual_inf;
        if (std::isnan(expected)) ++observation.reference_nan;
        if (std::isinf(expected)) ++observation.reference_inf;
        const float abs_error = std::fabs(actual - expected);
        const float rel_error = abs_error / std::max(std::fabs(expected), 1.0e-6f);
        if (std::isfinite(abs_error)) observation.max_abs = std::max(observation.max_abs, abs_error);
        else if (std::isinf(abs_error)) observation.max_abs = INFINITY;
        if (std::isfinite(rel_error)) observation.max_rel = std::max(observation.max_rel, rel_error);
        else if (std::isinf(rel_error)) observation.max_rel = INFINITY;
        if (IsMismatch(actual, expected, atol, rtol)) {
            ++observation.failures;
            if (observation.first_mismatch == std::numeric_limits<size_t>::max()) {
                observation.first_mismatch = index;
                observation.first_actual = actual;
                observation.first_reference = expected;
                observation.first_abs = abs_error;
                observation.first_rel = rel_error;
            }
        }
    }
    return observation.failures == 0 && observation.sync_status == ACL_SUCCESS;
}

void PrintInvrms(const CaseSpec& spec, int run, const Observation& observation)
{
    for (size_t row = 0; row < observation.reference_inv_rms.size(); ++row) {
        std::printf("DIAG_INVRMS source=%s run=%d case=%s row=%zu reference_inv_rms=%.9g actual_inv_rms=UNAVAILABLE\n",
                    UB_DIAG_SOURCE_LABEL, run, spec.name, row,
                    observation.reference_inv_rms[row]);
    }
}

void PrintObservation(const CaseSpec& spec, int run, const Observation& observation)
{
    const size_t tileCount = (static_cast<size_t>(spec.width) + kTileElems - 1) / kTileElems;
    const size_t tailElements = static_cast<size_t>(spec.width) % kTileElems;
    const bool hasTail = tailElements != 0;
    const size_t firstIndex = 0;
    const size_t lastIndex = static_cast<size_t>(spec.rows) * spec.width - 1;
    const float actualFirst = observation.actual_bytes.empty()
                                  ? std::numeric_limits<float>::quiet_NaN()
                                  : LoadValue(observation.actual_bytes, firstIndex, spec.dtype);
    const float actualLast = observation.actual_bytes.empty()
                                 ? std::numeric_limits<float>::quiet_NaN()
                                 : LoadValue(observation.actual_bytes, lastIndex, spec.dtype);
    const float referenceFirst = observation.reference.empty() ?
                                     std::numeric_limits<float>::quiet_NaN() : observation.reference[firstIndex];
    const float referenceLast = observation.reference.empty() ?
                                    std::numeric_limits<float>::quiet_NaN() : observation.reference[lastIndex];
    std::printf(
        "DIAG_CASE source=%s run=%d case=%s dtype=%d rows=%d width=%d blocks=1 tile_elems=%d tile_count=%zu tail=%s tail_elements=%zu launch=%s sync=%d copy=%s result=%s failures=%zu max_abs=%.9g max_rel=%.9g actual_nan=%zu actual_inf=%zu reference_nan=%zu reference_inf=%zu reference_inv_rms_min=%.9g reference_inv_rms_max=%.9g actual_inv_rms=UNAVAILABLE actual_first=%.9g reference_first=%.9g actual_last=%.9g reference_last=%.9g\n",
        UB_DIAG_SOURCE_LABEL, run, spec.name, spec.dtype, spec.rows, spec.width, kTileElems,
        tileCount, hasTail ? "TAIL" : "NON_TAIL", tailElements,
        observation.launch_ok ? "PASS" : "FAIL", observation.sync_status,
        observation.copy_ok ? "PASS" : "FAIL",
        (observation.failures == 0 && observation.sync_status == ACL_SUCCESS) ? "PASS" : "FAIL",
        observation.failures, observation.max_abs, observation.max_rel,
        observation.actual_nan, observation.actual_inf,
        observation.reference_nan, observation.reference_inf,
        observation.reference_inv_rms.empty() ? 0.0f : *std::min_element(observation.reference_inv_rms.begin(), observation.reference_inv_rms.end()),
        observation.reference_inv_rms.empty() ? 0.0f : *std::max_element(observation.reference_inv_rms.begin(), observation.reference_inv_rms.end()),
        actualFirst, referenceFirst, actualLast, referenceLast);
    if (observation.first_mismatch == std::numeric_limits<size_t>::max()) {
        std::printf("DIAG_FIRST_MISMATCH source=%s run=%d case=%s index=NONE actual_output=NONE reference_output=NONE\n",
                    UB_DIAG_SOURCE_LABEL, run, spec.name);
    } else {
        const size_t row = observation.first_mismatch / static_cast<size_t>(spec.width);
        const size_t col = observation.first_mismatch % static_cast<size_t>(spec.width);
        std::printf(
            "DIAG_FIRST_MISMATCH source=%s run=%d case=%s index=%zu row=%zu col=%zu tail=%s actual_output=%.9g reference_output=%.9g abs=%.9g rel=%.9g reference_inv_rms=%.9g actual_inv_rms=UNAVAILABLE\n",
            UB_DIAG_SOURCE_LABEL, run, spec.name, observation.first_mismatch, row, col,
            (col / kTileElems == tileCount - 1 && tailElements != 0) ? "TAIL" : "NON_TAIL",
            observation.first_actual, observation.first_reference, observation.first_abs,
            observation.first_rel, observation.reference_inv_rms[row]);
    }
    PrintInvrms(spec, run, observation);
}

void PrintRepeatability(const CaseSpec& spec, int run, const Observation& previous,
                        const Observation& current)
{
    const size_t elementBytes = ElementBytes(spec.dtype);
    size_t differingElements = 0;
    size_t firstDifferent = std::numeric_limits<size_t>::max();
    const size_t count = static_cast<size_t>(spec.rows) * spec.width;
    for (size_t index = 0; index < count; ++index) {
        const unsigned char* oldValue = previous.actual_bytes.data() + index * elementBytes;
        const unsigned char* newValue = current.actual_bytes.data() + index * elementBytes;
        if (std::memcmp(oldValue, newValue, elementBytes) != 0) {
            ++differingElements;
            if (firstDifferent == std::numeric_limits<size_t>::max()) firstDifferent = index;
        }
    }
    std::printf("DIAG_REPEATABILITY source=%s case=%s previous_run=%d current_run=%d status=%s differing_elements=%zu",
                UB_DIAG_SOURCE_LABEL, spec.name, run - 1, run,
                differingElements == 0 ? "IDENTICAL" : "DIFFERENT", differingElements);
    if (firstDifferent == std::numeric_limits<size_t>::max()) {
        std::printf(" first_diff_index=NONE\n");
    } else {
        std::printf(" first_diff_index=%zu previous_output=%.9g current_output=%.9g\n",
                    firstDifferent,
                    LoadValue(previous.actual_bytes, firstDifferent, spec.dtype),
                    LoadValue(current.actual_bytes, firstDifferent, spec.dtype));
    }
}

int RunDiagnostic(int device, int runs)
{
    std::printf("DIAG_BOOTSTRAP source=%s source_path=%s source_sha=%s device=%d runs=%d toolkit_expected=CANN_8.5.0.alpha002 soc=Ascend910B3 arch=dav-2201\n",
                UB_DIAG_SOURCE_LABEL, UB_DIAG_SOURCE_PATH, UB_DIAG_SOURCE_SHA, device, runs);
    std::printf("DIAG_PROTOCOL dtype0=FP32 storage_bytes=4 dtype1=FP16 storage_bytes=2 dtype2=BF16 storage_bytes=2 reference_rounding=FP32_raw_FP16_stagewise_BF16_final protocol_cases=8 tile_elems=%d timing=NOT_RUN online=NOT_RUN\n",
                kTileElems);
    const CaseSpec cases[] = {
        {"fp16-wide-16384", 1, 16384, kFp16},
        {"fp16-wide-16384-rows3", 3, 16384, kFp16},
        {"fp16-wide-32768", 1, 32768, kFp16},
        {"fp16-wide-32768-rows2", 2, 32768, kFp16},
        {"fp16-boundary-8192", 1, 8192, kFp16},
        {"fp16-tail-16385", 1, 16385, kFp16},
        {"bf16-wide-16384", 1, 16384, kBf16},
        {"fp32-wide-16384", 1, 16384, kFp32},
    };

    if (!CheckAcl(aclInit(nullptr), "acl_init") ||
        !CheckAcl(aclrtSetDevice(device), "set_device")) {
        return 2;
    }
    bool allPass = true;
    for (const CaseSpec& spec : cases) {
        Observation previous;
        bool havePrevious = false;
        for (int run = 1; run <= runs; ++run) {
            Observation current;
            const bool pass = RunCase(spec, device, 1, run, current);
            PrintObservation(spec, run, current);
            if (havePrevious && !previous.actual_bytes.empty() && !current.actual_bytes.empty()) {
                PrintRepeatability(spec, run, previous, current);
            }
            allPass = allPass && pass;
            previous = current;
            havePrevious = true;
        }
    }
    CheckAcl(aclFinalize(), "acl_finalize");
    std::printf("DIAG_COMPLETE source=%s result=%s timing=NOT_RUN online=NOT_RUN\n",
                UB_DIAG_SOURCE_LABEL, allPass ? "PASS" : "FAIL");
    return allPass ? 0 : 3;
}

}  // namespace ub_diag_2

int main(int argc, char** argv)
{
    if (argc < 2 || argc > 3) {
        std::fprintf(stderr, "usage: %s DEVICE [RUNS]\n", argv[0]);
        return 1;
    }
    const int device = std::atoi(argv[1]);
    const int runs = argc == 3 ? std::atoi(argv[2]) : 2;
    if (device < 0 || runs < 1 || runs > 8) {
        std::fprintf(stderr, "invalid device/runs: device=%d runs=%d\n", device, runs);
        return 1;
    }
    return ub_diag_2::RunDiagnostic(device, runs);
}
