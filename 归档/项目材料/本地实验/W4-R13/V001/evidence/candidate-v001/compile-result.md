# V001 Compile 记录

```text
HOST=cann-server3
TOOLCHAIN=CANN 8.5.0.alpha002
SOC=Ascend910B3 / dav-2201
COMMAND=cd /home/data4t2/lelinfeng/cann/server_runs/W4-R13/parent-tail-probe && bash build_server3.sh --target w4r13_ref_candidate_probe
RESULT=PASS
EXIT_CODE=0
TARGET=/home/data4t2/lelinfeng/cann/server_runs/W4-R13/parent-tail-probe/build/w4r13_ref_candidate_probe
```

Ascend C 编译与链接均完成。输出保留 runner 中既有的 `GM_ADDR` 属性忽略告警及 printf 宽度告警；未为这些告警改动 runner。
