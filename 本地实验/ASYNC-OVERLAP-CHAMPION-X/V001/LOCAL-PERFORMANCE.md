# V001 Local Performance

协议：`runner_ref.inc` 统一测量层（device-event 主、wall 次），warmup=45，samples=21，
device 4，2026-09-28。探针：`aoc_ref_parent_probe`（SHA a8c19a19…）与
`aoc_ref_v001_probe`（SHA 0fc71e8e…）。

## Same-binary

| shape | binary | B1 MAD/med | B2 MAD/med | drift | 资格 |
|---|---|---|---|---|---|
| 32×16384 fp16 | P (first) | 0.048 PASS | 0.474 FAIL | 1.96 | 不合格 |
| 32×16384 fp16 | P (retry) | 0.014 PASS | 0.346 FAIL | 1.54 | 不合格 |
| 32×16384 fp16 | C | 0.025 PASS | 0.057 PASS | 1.03 | 合格 |
| 16×32768 fp16 | P | 0.016 PASS | 0.169 FAIL | 1.21 | 不合格 |
| 16×32768 fp16 | C | 0.434 FAIL | 0.384 FAIL | — | 不合格 |

kernel 本体约 9–10 µs（32×16384），但同进程内偶发 60–340 µs 宿主离群点，
把 CV/MAD 顶爆。parent 的 B2 反复不过关。

## P/C 交错（32×16384 fp16，4 对）

| pair | P med / MAD | C med / MAD | Δmed% | Δp10% | 备注 |
|---|---|---|---|---|---|
| 1 | 17.54 / 7.86 | 9.38 / 0.22 | -46.5 | **-4.6** | P 脏 |
| 2 | 9.44 / 0.34 | 9.84 / 0.16 | +4.2 | **+6.0** | 双方干净 |
| 3 | 9.82 / 0.24 | 9.64 / 0.36 | -1.8 | **-3.8** | 双方干净 |
| 4 | 10.02 / 0.26 | 16.42 / 7.14 | +63.9 | **-5.4** | C 脏 |

p10（快簇）方向：3/4 偏 candidate（约 4–5%），1/4 反向。
干净对的 median 一正一负（+4.2 / -1.8）。
median 受离群点支配，不可用作主判定。

## 结论

```text
LOCAL_VERDICT     NEEDS_ONE_MORE_LOCAL
REASON            正确性 PASS_VS_PARENT；性能方向在 p10 快簇上 3/4 偏 V001（~4-5%），
                  与「每 batch 消掉一段串行边界」的预期量级同向，但 1/4 反向，
                  干净对 median 分裂，且 parent same-binary 不稳定合格。
                  落在 ~10µs 短 kernel 的噪声范围内，不能记 LOCAL_ACCEPTED。
NEXT              安静窗口下重测 32×16384，并加 64×16384（blocks=40 时部分核
                  localRows=2 → batchRows>1，prologue 占比更大）与 8×12288；
                  要求 parent same-binary 两块都过再 P/C。
```

原始样本：`本地实验/ASYNC-OVERLAP-CHAMPION-X/V001/results/*-raw.tsv`、`*-stats.txt`。
