# V002 Build + Correctness

## Build

| 项 | 值 |
|---|---|
| source SHA256 | `f8f65e09570e6db931d86fb1e4640599bc63b3fef2b42746e7065c8f75150c03` |
| compile device / submission | PASS / PASS |
| link device.alink | PASS `b36dd61dfba33cac9e15481e9b73126186f5469553b4f2cb4f7aa236093e3565` |
| link submission.alink | PASS `6acc58a8f4915e4634992ffe3c58d4f8fbdd18fc2f7d04d47c1e22137d2d83f1` |
| BUILD | PASS |

## Correctness（NPU，device 4，OUTHASH vs parent）

| shape | path | dtype | parent hash | cand hash | 同? | vs golden |
|---|---|---|---|---|---|---|
| 64×2048 | NarrowMid | fp32 | 68667a294de39908 | 68667a294de39908 | **是** | PASS |
| 64×2048 | NarrowMid | fp16 | 100b419edd74fcb0 | 100b419edd74fcb0 | **是** | bad=2 容差（双方相同） |
| 64×2048 | NarrowMid | bf16 | 52f7fcc0048eeee7 | 52f7fcc0048eeee7 | **是** | PASS |
| 32×4096 | NarrowMid | fp32 | fe50b2a35f2769cf | fe50b2a35f2769cf | **是** | PASS |
| 128×512 | NarrowMid | fp32 | b9790b09a714c53e | b9790b09a714c53e | **是** | PASS |
| 8×8192 | control wide | fp16 | 04b38d055725c9b1 | 04b38d055725c9b1 | **是** | bad=1 容差（双方相同） |
| 33×100 | control generic | fp32 | 9fe600dd1475d372 | 9fe600dd1475d372 | **是** | PASS |

```text
CORRECTNESS_STATUS   PASS_VS_PARENT
DETAIL               全部 7 形状 OUTHASH 与 parent 逐位一致；
                     含 NarrowMid 的 FP32/FP16/BF16（改动面）与 wide/generic 控制组。
GOLDEN_NOTE          FP16 宽行/中宽行的 1–2 元素 err=0.00293 为 harness 容差，
                     parent 与 candidate 同步出现，非 V002 回归。
```
