# InitSpmBuffer

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
- Atlas 推理系列产品AI Core：支持
<!-- end id5 -->
<!-- npu="310p" id6 -->
- Atlas 推理系列产品Vector Core：不支持
<!-- end id6 -->
<!-- npu="910" id7 -->
- Atlas 训练系列产品：支持
<!-- end id7 -->
<!-- npu="x90" id8 -->
- Kirin X90：支持
<!-- end id8 -->
<!-- npu="9030" id9 -->
- Kirin 9030：支持
<!-- end id9 -->
<!-- @ref: asc-devkit/res/docs/zh/api/SIMD-API/basic_api/resource_management/TPipe/InitSpmBuffer_res.md#id1 -->

## 功能说明

头文件路径为：`"basic_api/kernel_tpipe.h"`。

初始化SPM Buffer。SPM Buffer是一块在GM上的空间，当Unified Buffer（UB）空间不足时可以将溢出数据存放在SPM Buffer上。

## 函数原型

-   暂存到workspace初始化，需要指定GM地址为SPM Buffer：

    ```cpp
    template <typename T>
    __aicore__ inline void InitSpmBuffer(const GlobalTensor<T>& workspace, const int32_t bufferSize)
    ```

-   暂存到L1 Buffer初始化，不需要指定地址，会默认暂存到L1 Buffer，只需要传入需要的SPM Buffer大小：

    ```cpp
    __aicore__ inline void InitSpmBuffer(const int32_t bufferSize)
    ```

    <!-- npu="950" id10 -->
    Ascend 950PR/Ascend 950DT，不支持暂存到L1 Buffer初始化接口。
    <!-- end id10 -->
    <!-- npu="A3" id11 -->
    Atlas A3 训练系列产品/Atlas A3 推理系列产品，不支持暂存到L1 Buffer初始化接口。
    <!-- end id11 -->
    <!-- npu="910b" id12 -->
    Atlas A2 训练系列产品/Atlas A2 推理系列产品，不支持暂存到L1 Buffer初始化接口。
    <!-- end id12 -->

## 参数说明

**表1**  模板参数说明

| 参数名称 | 含义 |
| -------- | ---- |
| T | 待获取Tensor的数据类型，支持的类型请见[LocalTensor](../../data_structures/LocalTensor/LocalTensor.md)相关描述。|

**表2**  接口参数说明

| 参数名 | 输入/输出 | 含义 |
| --- | --- | --- |
| workspace | 输入 | workspace地址。 |
| bufferSize | 输入 | SPM Buffer的大小，单位是字节。 |

## 约束说明

SPM Buffer供解决UB空间不足的特殊场景使用，性能较差，一般不推荐使用。

## 返回值说明

无

## 调用示例

-   暂存到workspace初始化

    ```cpp
    AscendC::TPipe pipe;
    int len = 1024; // 设置spm buffer为1024个类型为T的数据
    workspace_gm.SetGlobalBuffer((__gm__ T *)usrWorkspace, len);  // 此处的usrWorkspace为用户自定义的workspace
    auto gm = workspace_gm[AscendC::GetBlockIdx() * len];
    pipe.InitSpmBuffer(gm, len * sizeof(T));
    ```

-   暂存到L1 Buffer初始化

    ```cpp
    AscendC::TPipe pipe;
    int len = 1024; // 设置spm buffer为1024个类型为T的数据
    pipe.InitSpmBuffer(len * sizeof(T));
    ```
