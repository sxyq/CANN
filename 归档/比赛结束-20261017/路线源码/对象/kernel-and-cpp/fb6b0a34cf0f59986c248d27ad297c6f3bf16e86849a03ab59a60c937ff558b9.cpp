#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "compile_adapter.hpp"

// Local mock of CANNJudge judge.asc: predefines ABI types then includes kernel.asc.

#include "add_rms_norm_bias_kernel.cpp"
