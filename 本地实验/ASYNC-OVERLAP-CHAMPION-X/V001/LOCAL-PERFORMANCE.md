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

---

# 补测附录（2026-09-28，MAIN-2 指令）

按指令补测 64×16384 fp16（batchRows>1）与 8×12288 fp16；要求 parent same-binary
两块均合格后再 P/C。协议同前（warmup=45, samples=21, blocks=2, device 4）。

## Same-binary 补测

| shape | binary | B1 MAD/med | B2 MAD/med | drift | 资格 |
|---|---|---|---|---|---|
| 64×16384 fp16 | P attempt1 | 0.022 PASS | 0.159 FAIL | 1.167 | 不合格 |
| 64×16384 fp16 | P attempt2 | 0.012 PASS | 0.181 FAIL | 1.174 | 不合格 |
| 64×16384 fp16 | C | 0.050 PASS | 0.152 FAIL | 1.111 | 不合格 |
| 8×12288 fp16 | P attempt1 | 0.025 PASS | 0.083 PASS | 1.179 | 不合格（drift） |
| 8×12288 fp16 | P attempt2 | 0.059 PASS | 0.376 FAIL | 1.594 | 不合格 |
| 8×12288 fp16 | C | 0.147 FAIL | 0.182 FAIL | 1.152 | 不合格 |

parent same-binary 两形状、两次尝试均未过两块 + drift 双条件。按指令**不硬测 P/C**。

## MEASUREMENT_BLOCKED 记录

```text
BLOCK_RECORD_1
  BLOCK_REASON         PARENT_SAME_BINARY_NOT_QUALIFIED: B2 MAD/med 0.159–0.181 > 0.10,
                       drift 1.167–1.174 > 1.10；~13µs kernel 宿主离群点污染
  DATE                 2026-09-28
  SHAPE                64x16384_fp16
  DEVICE               4
  SAME_BINARY_RESULT   P a1 B1 PASS(0.022)/B2 FAIL(0.159) drift 1.167;
                       P a2 B1 PASS(0.012)/B2 FAIL(0.181) drift 1.174;
                       C B1 PASS(0.050)/B2 FAIL(0.152) drift 1.111
  RETRY_REQUIRED       YES — 更安静窗口或换设备；parent B1+B2 均 PASS 后才 P/C

BLOCK_RECORD_2
  BLOCK_REASON         PARENT_SAME_BINARY_NOT_QUALIFIED: drift 1.179–1.594 > 1.10；
                       a2 B2 MAD/med 0.376；~7µs kernel 过短，两块采样不稳
  DATE                 2026-09-28
  SHAPE                8x12288_fp16
  DEVICE               4
  SAME_BINARY_RESULT   P a1 B1 PASS(0.025)/B2 PASS(0.083) drift 1.179 FAIL;
                       P a2 B1 PASS(0.059)/B2 FAIL(0.376) drift 1.594;
                       C B1 FAIL(0.147)/B2 FAIL(0.182) drift 1.152
  RETRY_REQUIRED       YES — 更安静窗口；7µs kernel 低于可靠测量长度
```

## 补测后总判定

```text
LOCAL_VERDICT     NEEDS_ONE_MORE_LOCAL（维持）
REASON            32×16384：p10 快簇 3/4 偏 V001（~4–5%）但 1/4 反向，干净对
                  median 分裂，parent SB 不稳定合格。
                  64×16384 / 8×12288：parent SB 两次均不合格 → MEASUREMENT_BLOCKED，
                  未硬测 P/C。
                  三个形状都拿不到「parent SB 双块合格 + 一致 P/C」的完整证据链。
                  主判定以规范主统计量（median/MAD）为准；p10 仅作次级诊断，
                  其 3/4 同向不足以定 LOCAL_ACCEPTED。
```

## 残余不确定性与下一步选项（不无限测）

| 选项 | 内容 | 建议 |
|---|---|---|
| A 同假设再测 | 更安静窗口（VLLM 空闲期 / 换 d5）重测 64×16384，要求 parent SB 双块 PASS | 若 Main 能给干净窗口，优先 |
| B 换形状 | 用更长 kernel 的 LP 形状（如 128×16384 或 32×32768）降低相对噪声 | kernel >20µs 时 B2 稳定性更好 |
| C 交 Main 定夺 | 接受当前证据为 NEEDS_ONE_MORE_LOCAL，保留 Candidate，不开 V002 | 若窗口长期不可得 |

不继续无限测；不 revert H1；不叠加新优化；不开 V002；不 Online。
