# Broadcast样例

## 概述

本样例基于Broadcast高阶API实现数据广播功能，支持将输入Tensor按照指定轴扩展到目标shape，适用于数据对齐和维度扩展等场景。

## 本样例支持的产品及CANN软件版本

| 产品 | CANN软件版本 |
|------|-------------|
| Ascend 950PR/Ascend 950DT | >= CANN 9.1.0 |
| Atlas A3 训练系列产品/Atlas A3 推理系列产品 | >= CANN 9.0.0 |
| Atlas A2 训练系列产品/Atlas A2 推理系列产品 | >= CANN 9.0.0 |

## 目录结构介绍

```text
├── broadcast
│   ├── scripts
│   │   └── gen_data.py         // 输入数据和真值数据生成脚本
│   ├── CMakeLists.txt          // 编译工程文件
│   ├── data_utils.h            // 数据读入写出函数
│   ├── broadcast.asc           // Ascend C样例实现 & 调用样例
│   └── README.md               // 样例说明文档
```

## 样例描述

- 样例功能：
  对输入Tensor做广播计算。

- 样例规格：

| 样例类型(OpType) | broadcast |  |  |  |
| --- | --- | --- | --- | --- |
| 样例输入 | name | shape | data type | format |
| x | [1, 48] | float | ND |  |
| 样例输出 | y | [96, 48] | float | ND |
| 核函数名 | broadcast_custom |  |  |  |

| 样例类型(OpType) | broadcast |  |  |  |
| --- | --- | --- | --- | --- |
| 样例输入 | name | shape | data type | format |
| x | [96, 1] | float | ND |  |
| 样例输出 | y | [96, 96] | float | ND |
| 核函数名 | broadcast_custom |  |  |  |

- 场景说明：


| TESTCASE | 输入shape | 输出shape | 广播轴(axis) | 说明 |
| --- | --- | --- | --- | --- |
| 0 | [1, 48] | [96, 48] | 0 | 在第一维进行广播，将1扩展为96 |
| 1 | [96, 1] | [96, 96] | 1 | 在第二维进行广播，将1扩展为96 |

- 样例实现：
  本样例中实现了两种场景的broadcast样例，分别是[1, 48]到[96, 48]的广播和[96, 1]到[96, 96]的广播。

  - Kernel实现
    使用Broadcast高阶API接口完成广播，将输入Tensor按照指定轴扩展到目标shape

  - Tiling实现
    broadcast样例的tiling实现流程如下：首先获取input和output的二维shape，然后将其与广播的轴、input/output tensor的维度填充到TilingData中。

  - 调用实现
    使用内核调用符<<<>>>调用核函数。

## 编译运行

在本样例根目录下执行如下步骤，编译并执行样例。

- 配置环境变量
  请根据当前环境上CANN开发套件包的[安装方式](../../../../../docs/zh/quick_start.md#prepare&install)，配置环境变量。
  ```bash
  source ${install_path}/cann/set_env.sh
  ```

  > **说明：** `${install_path}` 为CANN包安装目录，未指定安装目录时默认安装至 `/usr/local/Ascend` 下。

- 样例执行

  在本样例目录下执行如下命令。
  ```bash
  TESTCASE=1                    # 0: shape[1, 48]->[96,48]  1: shape[96,1]->[96,96]
  mkdir -p build && cd build;   # 创建并进入build目录
  cmake -DCMAKE_ASC_ARCHITECTURES=dav-2201 ..;make -j;    # 编译工程，默认npu模式
  python3 ../scripts/gen_data.py --testcase $TESTCASE  # 生成测试输入数据
  ./demo $TESTCASE              # 执行编译生成的可执行程序，执行样例
  ```

  使用 CPU调试 或 NPU仿真 模式时，添加 `-DCMAKE_ASC_RUN_MODE=cpu` 或 `-DCMAKE_ASC_RUN_MODE=sim` 参数即可。

  示例如下：

  ```bash
  cmake -DCMAKE_ASC_RUN_MODE=cpu -DCMAKE_ASC_ARCHITECTURES=dav-2201 ..;make -j; # cpu调试模式
  cmake -DCMAKE_ASC_RUN_MODE=sim -DCMAKE_ASC_ARCHITECTURES=dav-2201 ..;make -j; # NPU仿真模式
  ```

  > **注意：** 切换编译模式前需清理 cmake 缓存，可在 build 目录下执行 `rm CMakeCache.txt` 后重新 cmake。

- 编译选项说明

  | 选项 | 可选值 | 说明 |
  |------|--------|------|
  | `CMAKE_ASC_RUN_MODE` | `npu`（默认）、`cpu`、`sim` | 运行模式：NPU 运行、CPU调试、NPU仿真 |
  | `CMAKE_ASC_ARCHITECTURES` | `dav-2201`（默认）、`dav-3510` | NPU 架构：dav-2201 对应 Atlas A2 训练系列产品/Atlas A2 推理系列产品和 Atlas A3 训练系列产品/Atlas A3 推理系列产品，dav-3510 对应 Ascend 950PR/Ascend 950DT |

- 执行结果

  执行结果如下，说明精度对比成功。

  ```bash
  test pass!
  ```
