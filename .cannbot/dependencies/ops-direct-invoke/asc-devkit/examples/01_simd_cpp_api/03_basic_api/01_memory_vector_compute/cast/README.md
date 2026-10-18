# Cast样例

## 概述

本样例基于Cast实现源操作数和目的操作数Tensor的数据类型及精度转换。本样例实现half到int32_t、half到int4b_t两种场景下的转换。

## 本样例支持的产品及CANN软件版本

| 产品 | CANN软件版本 |
|------|-------------|
| Ascend 950PR/Ascend 950DT | >= CANN 9.1.0 |
| Atlas A3 训练系列产品/Atlas A3 推理系列产品 | >= CANN 9.0.0 |
| Atlas A2 训练系列产品/Atlas A2 推理系列产品 | >= CANN 9.0.0 |

## 目录结构介绍

```text
├── cast
│   ├── scripts
│   │   ├── gen_data.py         // 输入数据和真值数据生成脚本
│   │   └── verify_result.py    // 验证输出数据和真值数据是否一致的验证脚本
│   ├── CMakeLists.txt          // 编译工程文件
│   ├── data_utils.h            // 数据读入写出函数
│   ├── cast.asc                // Ascend C样例实现 & 调用样例
│   └── README.md               // 样例说明文档
```

## 样例描述

- 样例功能：
  CastCustom样例根据源操作数和目的操作数Tensor的数据类型进行精度转换。
- 样例规格：

| 样例类型(OpType) | cast |  |  |  |
| --- | --- | --- | --- | --- |
| 样例输入 |  |  |  |  |
| name | shape | data type | format |  |
| x | [1, 512] | half | ND |  |
| 样例输出 |  |  |  |  |
| y | [1, 512] | int32_t/int4b_t | ND |  |
| 核函数名 | cast_custom |  |  |  |

- 场景说明：

| SCENARIO_NUM | 输入类型 | 输出类型 | 说明 |
| --- | --- | --- | --- |
| 0 | half | int4b_t | half转int4b_t |
| 1 | half | int32_t | half转int32_t |

- 样例实现：
  本样例中实现的是固定shape为输入x[1, 512]，输出y[1, 512]的CastCustom样例，支持half到int32_t和half到int4b_t两种转换场景。

  - Kernel实现
    - 调用DataCopy基础API，将数据从GM（Global Memory）搬运到UB（Unified Buffer）
    - 调用Cast接口，进行数据类型转换（half转int32_t或half转int4b_t）
    - 调用DataCopy基础API，将结果从UB（Unified Buffer）搬运至GM（Global Memory）

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
  SCENARIO_NUM=0
  mkdir -p build && cd build;      # 创建并进入build目录
  cmake -DSCENARIO_NUM=$SCENARIO_NUM -DCMAKE_ASC_ARCHITECTURES=dav-2201 ..;make -j;    # 编译工程，默认npu模式
  python3 ../scripts/gen_data.py -scenarioNum=$SCENARIO_NUM  # 生成测试输入数据
  ./demo                        # 执行编译生成的可执行程序，执行样例
  python3 ../scripts/verify_result.py output/output.bin output/golden.bin   # 验证输出结果是否正确，确认算法逻辑正确
  ```

  使用 CPU调试 或 NPU仿真 模式时，添加 `-DCMAKE_ASC_RUN_MODE=cpu` 或 `-DCMAKE_ASC_RUN_MODE=sim` 参数即可。

  示例如：

  ```bash
  cmake -DSCENARIO_NUM=$SCENARIO_NUM -DCMAKE_ASC_RUN_MODE=cpu -DCMAKE_ASC_ARCHITECTURES=dav-2201 ..;make -j; # cpu调试模式
  cmake -DSCENARIO_NUM=$SCENARIO_NUM -DCMAKE_ASC_RUN_MODE=sim -DCMAKE_ASC_ARCHITECTURES=dav-2201 ..;make -j; # NPU仿真模式
  ```

  > **注意：** 切换编译模式前需清理 cmake 缓存，可在 build 目录下执行 `rm CMakeCache.txt` 后重新 cmake。

- 编译选项说明

  | 选项 | 可选值 | 说明 |
  |------|--------|------|
  | `CMAKE_ASC_RUN_MODE` | `npu`（默认）、`cpu`、`sim` | 运行模式：NPU 运行、CPU调试、NPU仿真 |
  | `CMAKE_ASC_ARCHITECTURES` | `dav-2201`（默认）、`dav-3510` | NPU 架构：dav-2201 对应 Atlas A2 训练系列产品/Atlas A2 推理系列产品和 Atlas A3 训练系列产品/Atlas A3 推理系列产品，dav-3510 对应 Ascend 950PR/Ascend 950DT |
  | `SCENARIO_NUM` | `0`（默认）、`1` | 场景：0 对应half转int4b_t，1 对应half转int32_t |

- 执行结果

  执行结果如下，说明精度对比成功。

  ```bash
  test pass!
  ```
