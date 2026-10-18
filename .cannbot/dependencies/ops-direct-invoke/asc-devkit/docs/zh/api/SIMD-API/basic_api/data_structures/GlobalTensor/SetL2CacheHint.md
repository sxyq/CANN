# SetL2CacheHint

## 产品支持情况

<!-- npu="950" id1 -->
- Ascend 950PR/Ascend 950DT：支持
<!-- end id1 -->
<!-- npu="A3" id2 -->
- Atlas A3 训练系列产品/Atlas A3 推理系列产品：支持
<!-- end id2 -->
<!-- npu="910b" id3 -->
- Atlas A2 训练系列产品/Atlas A2 推理系列产品：支持
<!-- end id3 -->
<!-- npu="310b" id4 -->
- Atlas 200I/500 A2 推理产品：不支持
<!-- end id4 -->
<!-- npu="310p" id5 -->
- Atlas 推理系列产品AI Core：不支持
<!-- end id5 -->
<!-- npu="310p" id6 -->
- Atlas 推理系列产品Vector Core：不支持
<!-- end id6 -->
<!-- npu="910" id7 -->
- Atlas 训练系列产品：不支持
<!-- end id7 -->
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/data_structures/GlobalTensor/SetL2CacheHint_res.md#id1 -->

## 功能说明

设置GlobalTensor是否启用L2 Cache，默认启用L2 Cache。

## 函数原型

```cpp
template<CacheRwMode rwMode = CacheRwMode::RW>
__aicore__ inline void SetL2CacheHint(CacheMode mode);
```

## 参数说明

**表1**  模板参数说明

| 参数名 | 描述 |
| --- | --- |
| rwMode | 设置L2 Cache读写模式。
enum CacheRwMode {
READ = 1,
WRITE = 2,
RW = 3
};
预留参数。为后续的功能做保留，开发者暂时无需关注，使用默认值即可。 |

**表2**  参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| mode | 输入 | 用户指定的L2 Cache模式。
enum class CacheMode : uint8_t {
CACHE_MODE_DISABLE = 0, // 不启用L2 Cache
CACHE_MODE_NORMAL = 1,  // 启用L2 Cache
CACHE_MODE_PERSISTENT = 4,  // 启用L2 Cache驻留模式
};
当特定GlobalTensor启用L2 Cache后实测性能反而下降时，可考虑手动禁用该GlobalTensor的L2 Cache功能。例如，若某算子仅对特定GlobalTensor执行单次读取操作，将其数据缓存至L2 Cache不仅无法带来性能收益，反而可能因数据频繁搬运至L2 Cache而引入额外的开销，此时建议关闭该GlobalTensor的L2 Cache。
通常情况下，L2 Cache可采用CACHE_MODE_NORMAL模式运行。在此模式下，当L2 Cache容量耗尽时会触发数据置换机制，已存入L2 Cache中的数据可能被替换。若需确保特定GlobalTensor的数据始终保留在L2 Cache中，可采用驻留模式。目前该驻留模式功能尚在开发中，暂不支持，计划于Ascend 950PR/Ascend 950DT产品上提供支持。
如果不调用该接口，默认为CacheMode::CACHE_MODE_NORMAL，即GlobalTensor会启用L2 Cache。 |

## 返回值说明

无。

## 约束说明

无

## 调用示例

```cpp
uint64_t dataSize = 256; //设置input_global的大小为256

AscendC::GlobalTensor<int32_t> inputGlobal; // 类型为int32_t
inputGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ int32_t *>(src_gm), dataSize); // 设置源操作数在Global Memory上的起始地址为src_gm，所占外部存储的大小为256个int32_t
inputGlobal.SetL2CacheHint(AscendC::CacheMode::CACHE_MODE_DISABLE); // 设置GlobalTensor不会写入L2 Cache

AscendC::LocalTensor<int32_t> inputLocal = inQueueX.AllocTensor<int32_t>();
AscendC::DataCopy(inputLocal, inputGlobal, dataSize); // 将Global Memory上的inputGlobal拷贝到Local Memory的inputLocal上
```
