# API-PROBE-RESULT — DataCopyParams 单位语义（V002 实现前置）

日期: 2026-09-28，server3 d4
探针: `api_probe_kernel.asc` + `api_probe_host.asc`（GM→UB→GM，`DataCopy`+`DataCopyParams(1, field, 0, 0)`）
结果文件: `support/api_probe_result.txt`（server3 同名文件已回收）

| blockLen 字段值 | 若 32B 单位应复制 | 若字节单位应复制 | 实测复制（前缀精确匹配） | 尾部多写 |
|---|---|---|---|---|
| 4 | 128 B | 4 B | **128 B** | 0 |
| 8 | 256 B | 8 B | **256 B** | 0 |
| 32 | 1024 B | 32 B | **1024 B** | 0 |
| 64 | 2048 B | 64 B | **2048 B** | 0 |
| 256 | 8192 B | 256 B | **8192 B** | 0 |

## 结论

1. `DataCopyParams.blockLen` 单位 = **32 字节**。字段值 N 传输 32N 字节。
2. 对任意 32B 整数倍长度 L，`blockLen = L/32` 可精确表达（L≤2,097,120 B 均在 uint16 范围内；本 kernel 最大单次 tile 传输 128KB 量级，安全）。
3. 精确传输能力满足 V002 范围：`count*sizeof(T)%32==0` 的对齐点可安全走非 Pad `DataCopy`；非对齐长度无法用该 API 表达（会归并到 32B 粒度），保持 `DataCopyPad`——与 V002 设计一致。
4. 探针往返无尾部多写（tail_mismatch=0），语义干净。

**判定：API 可行，V002 继续实现。无需停下报 Main-2。**
