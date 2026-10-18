# Curated Devkit Snippets

This file binds representative AscendC code fragments to the skill.

Each section contains:

- when to use it
- what to avoid
- source path
- a minimal reusable fragment

Do not bulk-load the full example file unless the fragment here is insufficient.

These snippets are structural starting points, not API authority. Before copying or adapting a call into kernel code, verify every newly introduced API, overload, dtype, tensor position, parameter structure, and target-architecture use against `references/08_api_lookup.md` and the current `${DEVKIT_PATH}/docs/api` tree.

## 最小启动模板

Use when creating a first runnable kernel or a smoke-test launch path.

Avoid using it as the final structure for complex kernels. For real compute, switch to the queue or helper patterns below.

Source under the user-provided repository:
`${DEVKIT_PATH}/examples/01_simd_cpp_api/00_introduction/00_quickstart/hello_world_npu/hello_world.asc`

```cpp
__global__ __vector__ void hello_world()
{
    AscendC::printf("[Block (%lu/%lu)]: Hello World!!!\n",
        AscendC::GetBlockIdx(), AscendC::GetBlockNum());
}

int main()
{
    aclInit(nullptr);
    int32_t deviceId = 0;
    aclrtSetDevice(deviceId);
    aclrtStream stream = nullptr;
    aclrtCreateStream(&stream);

    constexpr uint32_t numBlocks = 8;
    hello_world<<<numBlocks, nullptr, stream>>>();
    aclrtSynchronizeStream(stream);

    aclrtDestroyStream(stream);
    aclrtResetDevice(deviceId);
    aclFinalize();
    return 0;
}
```

## 向量流水模板

Use when implementing a standard SIMD C++ API kernel with GM to Local copy, compute, and copy-back.

Avoid placing heavy logic directly inside the `__global__` entry.

Source under the user-provided repository:
`${DEVKIT_PATH}/examples/01_simd_cpp_api/00_introduction/01_vector/basic_api_tque_add/add.asc`

```cpp
class KernelAdd {
public:
    __aicore__ inline void Init(GM_ADDR x, GM_ADDR y, GM_ADDR z,
        uint32_t totalLength, uint32_t tileNum)
    {
        blockLength = totalLength / AscendC::GetBlockNum();
        tileLength = blockLength / tileNum / BUFFER_NUM;
        xGm.SetGlobalBuffer((__gm__ float *)x + blockLength * AscendC::GetBlockIdx(), blockLength);
        yGm.SetGlobalBuffer((__gm__ float *)y + blockLength * AscendC::GetBlockIdx(), blockLength);
        zGm.SetGlobalBuffer((__gm__ float *)z + blockLength * AscendC::GetBlockIdx(), blockLength);
        pipe.InitBuffer(inQueueX, BUFFER_NUM, tileLength * sizeof(float));
        pipe.InitBuffer(inQueueY, BUFFER_NUM, tileLength * sizeof(float));
        pipe.InitBuffer(outQueueZ, BUFFER_NUM, tileLength * sizeof(float));
    }

    __aicore__ inline void Process()
    {
        int32_t loopCount = tileNum * BUFFER_NUM;
        for (int32_t i = 0; i < loopCount; ++i) {
            CopyIn(i);
            Compute(i);
            CopyOut(i);
        }
    }

private:
    __aicore__ inline void CopyIn(int32_t progress) { /* DataCopy GM -> Local */ }
    __aicore__ inline void Compute(int32_t progress) { /* AscendC::Add */ }
    __aicore__ inline void CopyOut(int32_t progress) { /* DataCopy Local -> GM */ }
};

__global__ __vector__ void add_custom(GM_ADDR x, GM_ADDR y, GM_ADDR z, AddCustomTilingData tiling)
{
    KernelAdd op;
    op.Init(x, y, z, tiling.totalLength, tiling.tileNum);
    op.Process();
}
```

## `__aicore__` Helper 模板

Use when a kernel needs device-side utility logic such as tiling copy, ceiling helpers, or block-offset computation.

Avoid duplicating this logic inline across multiple kernels.

Source under the user-provided repository:
`${DEVKIT_PATH}/examples/01_simd_cpp_api/00_introduction/02_matrix/matmul/matmul.asc`

```cpp
__aicore__ inline uint32_t Ceiling(uint32_t a, uint32_t b)
{
    return (a + b - 1) / b;
}

__aicore__ inline void CopyTiling(TCubeTiling *tiling, uint64_t &localMemSize, GM_ADDR tilingGM)
{
    uint32_t *ptr = reinterpret_cast<uint32_t *>(tiling);
    auto tiling32 = reinterpret_cast<__gm__ uint32_t *>(tilingGM);
    for (uint32_t i = 0; i < sizeof(TCubeTiling) / sizeof(uint32_t); ++i, ++ptr) {
        *ptr = *(tiling32 + i);
    }
    localMemSize = *reinterpret_cast<__gm__ uint64_t *>(tilingGM + sizeof(TCubeTiling));
}

__aicore__ inline void CalcGMOffset(int blockIdx, const TCubeTiling &tiling,
    int &offsetA, int &offsetB, int &offsetC, int &tailM, int &tailN,
    bool isTransA, bool isTransB)
{
    uint32_t mSingleBlocks = Ceiling(tiling.M, tiling.singleCoreM);
    uint32_t mCoreIndx = blockIdx % mSingleBlocks;
    uint32_t nCoreIndx = blockIdx / mSingleBlocks;
    // compute offsets and tails here
}
```

## Matmul 与 Tiling 模板

Use when the task needs workspace, matmul library objects, or host-generated tiling.

Avoid inventing tiling layout manually when the repository tiling APIs already provide it.

Source under the user-provided repository:
`${DEVKIT_PATH}/examples/01_simd_cpp_api/00_introduction/02_matrix/matmul/matmul.asc`

```cpp
__global__ __aicore__ void matmul_custom(GM_ADDR a, GM_ADDR b, GM_ADDR c,
    __kfc_workspace__ GM_ADDR workspace, GM_ADDR tilingGm)
{
    AscendC::TPipe pipe;
    TCubeTiling tiling;
    uint64_t localMemSize = 0;
    CopyTiling(&tiling, localMemSize, tilingGm);

    AscendC::GlobalTensor<half> aGlobal;
    AscendC::GlobalTensor<half> bGlobal;
    AscendC::GlobalTensor<float> cGlobal;
    aGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ half *>(a), tiling.M * tiling.Ka);
    bGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ half *>(b), tiling.Ka * tiling.N);
    cGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ float *>(c), tiling.M * tiling.N);

    int offsetA = 0, offsetB = 0, offsetC = 0;
    int tailM = 0, tailN = 0;
    bool isTransA = false, isTransB = false;
    CalcGMOffset(GetBlockIdx(), tiling, offsetA, offsetB, offsetC, tailM, tailN, isTransA, isTransB);

    auto gmA = aGlobal[offsetA];
    auto gmB = bGlobal[offsetB];
    auto gmC = cGlobal[offsetC];
    // create matmul object, bind tensors, iterate, end
}

void GenerateTiling(const char *socVersion, uint8_t *tilingBuf)
{
    auto ascendcPlatform = platform_ascendc::PlatformAscendCManager::GetInstance(socVersion);
    matmul_tiling::MultiCoreMatmulTiling tilingApi(*ascendcPlatform);
    // SetAType / SetBType / SetCType / SetShape / GetTiling
}
```

## Debug 与 Dump 模板

Use when a kernel needs light instrumentation, tensor dumps, or example debug output.

Avoid assuming every buffer position is supported by stock dump APIs on every architecture.

Sources under the user-provided repository:
- `${DEVKIT_PATH}/examples/01_simd_cpp_api/01_utilities/00_printf/printf.asc`
- `${DEVKIT_PATH}/examples/01_simd_cpp_api/01_utilities/02_dump/mmad_custom_dump.asc`

```cpp
__aicore__ inline void CopyTiling(TCubeTiling *tiling, GM_ADDR tilingGM)
{
    uint32_t *ptr = reinterpret_cast<uint32_t *>(tiling);
    auto tiling32 = reinterpret_cast<__gm__ uint32_t *>(tilingGM);
    for (uint32_t i = 0; i < sizeof(TCubeTiling) / sizeof(uint32_t); ++i, ++ptr) {
        *ptr = *(tiling32 + i);
    }
    AscendC::printf("printf pointer %p\n", ptr);
}

__aicore__ inline void DumpExample(AscendC::GlobalTensor<half> &aGM,
    AscendC::LocalTensor<float> &c1Local)
{
    asc_dump_gm<half>((__gm__ half *)aGM.GetPhyAddr(), 1, 32);
    asc_dump_cbuf<float>((__cc__ float *)c1Local.GetPhyAddr(), 4, 32);
}
```

## A1 L0A L0B 访存路径模板

Use when the task explicitly involves L1, L0A, or L0B movement, or when a debug plan needs a storage-path reference.

Avoid treating this as a generic dump solution. It is a memory-path reference, not a universal debug API.

Source under the user-provided repository:
`${DEVKIT_PATH}/examples/01_simd_cpp_api/05_compatibility_guide/data_copy_l1togm_950/data_copy_l1togm.asc`

```cpp
__aicore__ inline void CopyGmToA1()
{
    AscendC::LocalTensor<half> leftMatrix = inQueueA1.AllocTensor<half>();
    AscendC::Nd2NzParams params{1, 64, 128, 0, 128, 64, 1, 0};
    AscendC::DataCopy(leftMatrix, aGlobal, params);
    inQueueA1.EnQue(leftMatrix);
}

__aicore__ inline void Load2DA1ToL0A()
{
    AscendC::LocalTensor<half> a1 = inQueueA1.DeQue<half>();
    AscendC::LocalTensor<half> a2 = inQueueA2.AllocTensor<half>();
    AscendC::LoadData2DParamsV2 loadDataParams;
    loadDataParams.mStep = AscendC::DivCeil(m, 16);
    loadDataParams.kStep = AscendC::DivCeil(k * sizeof(half), 32);
    AscendC::LoadData(a2, a1, loadDataParams);
    inQueueA2.EnQue(a2);
}
```
