# Task Goal Format Example

Use this bundled example as the required `TASK_GOAL` structure reference. This file is a neutral `TASK_GOAL` four-section input example, not a debug-bus or one-off requirement record. 这是一份中性的 `TASK_GOAL` 四段式输入样例。The headings are mandatory for AscendC requirement documents and requirement-code implementation tasks:

```md
## **需求**
## **背景简述**
## **涉及领域**
## **方案详述**
```

The concrete content below is a neutral example payload. Reuse its structure for unrelated tasks; reuse its technical details only when the user's task actually matches the same vector-pipeline requirement.

## Example

## **需求**
为一个 AscendC 向量基础 API 增加新的重载，并补充最小编译验证方案。

## **背景简述**
目标 API 已在 `include/basic_api/` 暴露，核心实现位于 `impl/basic_api/`。现有实现已经支持标准 `LocalTensor` 输入输出，但缺少 count-based 简化接口。目标 SoC 由用户提供，例如 `dav-2201`。

## **涉及领域**
AscendC basic API、Host/Device 调用层级、vector pipeline、编译验证。

## **方案详述**
1. 在 `DEVKIT_PATH` 下定位目标 API 的声明、实现和相邻同类 API。
2. 复用已有 `__aicore__ inline` 实现风格，保持 `__global__` 入口只做薄封装，Host 侧只通过 `<<<>>>` 启动 kernel。
3. 对 dtype、mask、repeat 或临时 buffer 的支持范围，以当前代码、官方文档和 `asc-npu-arch` 事实为准，不复制完整芯片或 dtype 表。
4. 使用已解析的 `CANN_PATH` 和 `SOC_ARCH` 做最小 `.asc` 编译验证；如果涉及仓内回归，再读取 `build.sh`、`tests/test_parts.sh` 和相关 `tests/**/CMakeLists.txt` 选择最小测试。
