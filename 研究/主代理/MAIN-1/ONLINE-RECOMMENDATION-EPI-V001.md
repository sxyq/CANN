# ONLINE_RECOMMENDATION — EPILOGUE-ARITH-CHAMPION-X V001

- Date: 2026-09-29
- Recommender: MAIN-1
- Route: EPILOGUE-ARITH-CHAMPION-X
- Revision: V001
- Direct Parent: R31B-V011（Official 45.16）
- Source SHA: `eda335d16e7e311b279e8918c6405c622bb7022e040459143f19a6240a546dbf`

## Recommendation: WORTHY

## Main Review ruling on acceptance band

Agent 提交判定包请示：以「形状自身配对噪声」还是「对照核散布」为验收带。

**Main 裁定：以形状自身配对噪声为正式验收带，对照核用于检测系统偏移。**

理由：协议口径是 paired delta 相对该形状的噪声范围；对照核（同码）用于发现候选槽位偏差/系统偏移。窗口 3 已把系统偏移压至 +1.00%（目标 ±1.5% 达成），主探针信号超出形状自身噪声 2.8×。对照核 pair-to-pair 散布宽是短核固有属性，不改变主探针判定。差分读法（−3.55pp）作补充证据保留。

→ **LOCAL_ACCEPTED 成立。**

## Trigger

| 条件 | 状态 | 证据 |
|---|---|---|
| 明显超过 noise floor | 达标（2.8× 形状噪声；系统偏移 +1.00%） | 1x32768 −2.55% |
| 多 pair / 相关 shape 方向一致 | YES | 三窗 pooled 14/16 p=0.002；1x32768 + 2x16384 同向 |
| 机制幅度同量级 | YES | 差分 −3.55pp vs 预测 3.5–8.1% |

信号强度弱于 R31B V016 / R31A V026，但证据结构完整。`ONLINE_DECISION = APPROVED_ON_TRIGGER`。

## Online 优先级（更新）

1. R31B V016（Champion 血统）
2. R31A V026（chain −8.3%）
3. **EPILOGUE-ARITH V001**（本推荐，NORM-HOIST）
4. R31A V025 / V024（可选）

## Online package

- Source: `线上结果/EPILOGUE-ARITH-CHAMPION-X/V001/submission.asc`
- SHA256: `eda335d16e7e311b279e8918c6405c622bb7022e040459143f19a6240a546dbf`
