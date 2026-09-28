# V001 Build + Correctness

## Build

| 项 | 值 |
|---|---|
| workspace | server3 `/home/data4t2/lelinfeng/phase4-workspaces/ASYNC-OVERLAP-CHAMPION-X/V001` |
| source SHA256 | `0fc71e8e62e39ed3b894b7e8fb8cabb6372f802c0a4c342eeb246629f8e4efe4` |
| compile device | PASS (bisheng, CMake device target) |
| compile submission | PASS |
| link device.alink | PASS `620550fe17e016b1e1a418d7693a02f49a0de1fb623c04f95fe7fa97628f86af` |
| link submission.alink | PASS `41faa98ae210516ab320cfec565bee775cf10340f53294d9d9ce50d69be2e2de` |
| BUILD | PASS |

## Correctness（NPU，device 4）

Harness：`support/runner_correctness.asc` ASC 可执行，链接 tiling_api/register/platform/unified_dlog。
同 harness 同时编译 parent（SHA a8c19a19…）与 candidate 做 OUTHASH 逐位对照。

| shape | path | dtype | parent hash | cand hash | 同? | vs golden |
|---|---|---|---|---|---|---|
| 8×16384 | LP primary tc4 | fp16 | 84a417650ca9e0cc | 84a417650ca9e0cc | **是** | bad=2 err=0.00293（双方相同） |
| 4×32768 | LP primary tc8 | fp16 | c7966b828d7a9dea | c7966b828d7a9dea | **是** | bad=1 err=0.00293（双方相同） |
| 8×16384 | LP primary tc4 | bf16 | 9574ed47d1b365db | 9574ed47d1b365db | **是** | PASS |
| 8×12288 | LP small tc3 | fp16 | ca4f89b24597c363 | ca4f89b24597c363 | **是** | PASS |
| 8×8192 | control 非 LP | fp16 | 04b38d055725c9b1 | 04b38d055725c9b1 | **是** | bad=1 err=0.00293（双方相同） |
| 4×16384 | control FullCache | fp32 | 变动 | 变动 | 否* | 双方都 FAIL（已知非确定） |
| 33×100 | control narrow | fp32 | 9fe600dd1475d372 | 9fe600dd1475d372 | **是** | PASS |

\* FullCache FP32 宽行：parent 自身多次运行 hash/bad 计数就不同（registry 已记录
「pre-existing parent/harness mismatch on wide FP32」）。该路径 V001 未改动。

## Correctness 结论

```text
CORRECTNESS_STATUS          PASS_VS_PARENT
CORRECTNESS_DETAIL          LP 路径（本 Revision 唯一改动面）全部形状 OUTHASH 与 parent 逐位一致；
                            BF16 LP 与 LP-tc3 / narrow 控制直接过 golden。
GOLDEN_TOLERANCE_NOTE       FP16 宽行 golden 容差 2.5e-3 而 parent/cand 同步出现
                            err=0.00293 的 1–2 个元素；双方 mismatch 指纹完全一致，
                            属 harness/容差问题，不是 V001 回归。
PRE_EXISTING_NONDET         FullCache FP32 4×16384 在 parent 上多次运行结果互相不同；
                            不作为 V001 回归，也不作主正确性门。
```

按执行约定：LP 路径无新增数值错误，可进入 same-binary / P/C。
