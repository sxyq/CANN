# 可运行参考候选

## 来源

- 来源：用户在当前会话提供的完整 Ascend C 源码。
- 本地文件：`kernel.asc`
- 原始附件：`/Users/sunyiyang/.codex/attachments/1c3b9209-0db0-4b74-ba6d-713e02319bc7/pasted-text.txt`
- 用户说明：该版本可以正常运行。

## 文件特征

- UTF-8 编码。
- CRLF 换行。
- 2947 行，160461 字节。
- CANNJudge 直调入口：`__global__ __vector__` Kernel 和 `extern "C" run_kernel`。
- 支持 FP32、FP16、BF16，并将输入的前置维度展平为行、沿最后一维处理。

## 与提交版本的关系

该文件与 `提交/混合方案/H001-正确性优先/V002/kernel.asc` 使用同一类主要算法和入口，但存在实际源码差异：

1. 本文件使用 `AscendC::TPipe pipe_`，V002 使用 `AscendC::TPipe tpipe`。
2. 本文件的 `Load` 使用未显式填写字段的 `DataCopyPadExtParams<T>`；V002 增加了尾块对齐和补零字段。

因此，本目录内容只作为独立参考候选。任何基于它的新提交版本都必须新建 `V00N`，并单独记录编译、精度和性能结果。

## 证据边界

用户已说明该源码可以运行；本机没有 CANN 工具链和昇腾 NPU，当前没有在本机独立复现。服务器或 CANNJudge 的结果必须以对应日志或提交页面记录为准。
