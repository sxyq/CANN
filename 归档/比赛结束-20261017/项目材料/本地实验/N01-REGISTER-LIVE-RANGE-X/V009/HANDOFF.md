# N01 V009 Handoff

V009 source is committed as `4298b511a1407d8bfff9f00054efedb3d7bef522`, Source SHA256 `75fd82bbd9dd9b97bb6f47233c600f871bffe1338632638db2035e4ab662c450`. The only source change scopes the Pass 1 `reduceLocal` view to the Pass 1 block.

- CANN 8.5 configure/build: PASS, exit `0`.
- Generated Runner and `.aicore_binary` are byte-identical to V008; no device-code change was observed.
- Correctness: PASS, 4/4 on d5.
- Paired Local: 4 x 21 pairs on d5; all raw values and outliers retained. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`.
- Full evidence and paired statistics: `RESULTS.md` and `results/`.

NEXT_ACTION: after committing and pushing V009 evidence, start V010 with a different single-factor FP32 temporary lifetime change; keep d5 as the only N01 device.
