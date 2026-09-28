#pragma once

#include "probe_prelude.inc"

using PairedHostEntry = void (*) (
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, int64_t, aclrtStream, float);

extern "C" bool r31a_run_kernel_v016(
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, int64_t, aclrtStream, float);

extern "C" bool r31a_run_kernel_v021(
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, int64_t, aclrtStream, float);
