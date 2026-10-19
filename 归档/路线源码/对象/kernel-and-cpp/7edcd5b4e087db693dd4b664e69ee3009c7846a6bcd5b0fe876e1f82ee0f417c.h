
#ifndef HEADER_ACLRTLAUNCH_ADD_RMS_NORM_BIAS_CUSTOM_HKERNEL_H_
#define HEADER_ACLRTLAUNCH_ADD_RMS_NORM_BIAS_CUSTOM_HKERNEL_H_



template<typename T>
uint32_t aclrtlaunch_add_rms_norm_bias_custom(uint32_t blockDim, void* stream, void* x, void* residual, void* gamma, void* bias, void* output, uint64_t rowCount, uint64_t rowWidth, uint32_t blockCount, float invRowWidth, float epsilon);

template<typename T>
inline uint32_t add_rms_norm_bias_custom(uint32_t blockDim, void* hold, void* stream, void* x, void* residual, void* gamma, void* bias, void* output, uint64_t rowCount, uint64_t rowWidth, uint32_t blockCount, float invRowWidth, float epsilon)
{
    (void)hold;
    return aclrtlaunch_add_rms_norm_bias_custom<T>(blockDim, stream, x, residual, gamma, bias, output, rowCount, rowWidth, blockCount, invRowWidth, epsilon);
}

#endif
