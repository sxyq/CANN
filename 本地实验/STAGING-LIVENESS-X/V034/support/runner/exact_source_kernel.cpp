#include "kernel_entry_abi.hpp"

#ifndef KERNEL_SOURCE_HEADER
#error KERNEL_SOURCE_HEADER must point to the pinned exact source
#endif

#define ASCENDC_CPU_DEBUG 1
#include KERNEL_SOURCE_HEADER
#undef ASCENDC_CPU_DEBUG
