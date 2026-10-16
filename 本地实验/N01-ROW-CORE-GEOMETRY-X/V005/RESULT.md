# N01 ROW-CORE-GEOMETRY-X V005

## HYPOTHESIS

在宽行 FP16 路径中，第一遍把 `x + residual` 转成 `valueFp32`，再将平方写到 `xFp32`；第二遍又用 `valueFp32` 暂存归一化结果。两遍不并行，`xFp32` 可在各阶段复用。若让转换、原位平方和第二遍归一化共用 `xFp32Buf_`，可少保留一个宽行 FP16 专用 FP32 tile，并减轻 Vector 临时值驻留。

这是假设，不代表寄存器分配或性能已有改善证据。V001、V002、V003 分别改变每核目标行数为 2、4、3，宽行结果均无收益；V005 不再改 launch block 数量或设备侧行归属。

## SOURCE BASIS

- Parent：`本地实验/N01-ROW-CORE-GEOMETRY-X/V002/parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc`。
- 上述 Parent 与 `V003/parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc` 内容相同；V003 的 `parent/` 保持原样。
- `ProcessWideLowPrecision` 第一遍原先用 `valueFp32Buf_` 接收转换结果，再写入 `xFp32Buf_` 做平方；第二遍原先又用 `valueFp32Buf_` 接收 `yTile` 并归一化。
- 同一宽行 FP16 分支中，第一遍归约和第二遍输出顺序执行。`xFp32Buf_` 在两处可复用，BF16 分支仍需 `valueFp32Buf_` 保存整批 FP32 y。

## ONE CHANGE

- Candidate：`本地实验/N01-ROW-CORE-GEOMETRY-X/V005/candidate/N01-ROW-CORE-GEOMETRY-H4_kernel.asc`。
- 仅改宽行 FP16 数据通路：`ToFloat` 与原位平方改用 `xFp32`；第二遍归一化也改用 `xFp32`；移除宽行 FP16 专用的 `valueFp32Buf_` 分配。
- BF16、FP32、窄行路径、算术顺序、FP16 输出舍入、tile 宽度、行分组与同步均保持不变。
- 未新增或修改 Runner、CMake、脚本或独立记录表。

## COMPILE

- `COMPILE=NOT_RUN`。
- 当前主机为 Darwin arm64，PATH 中没有 `cmake`。已有 CMake 入口面向 Ascend Linux 工具链；历史构建在 server3 完成。本轮限定在 N01 工作目录，未到其他目录构建。
- Vector 编译后的寄存器分配情况仍未知。

## CORRECTNESS / LOCAL

- `CORRECTNESS=NOT_RUN`；`LOCAL_SCORE=NONE`。
- 本轮没有执行任何 NPU 正确性或性能运行，符合任务范围。

## NEXT DEVICE ENTRY

沿用 `本地实验/N01-ROW-CORE-GEOMETRY-X/V003/paired/paired_runner.cpp` 与 `paired/CMakeLists.txt`。现有 `paired_runner_n01.asc` 候选 include 指向 H3；后续接入 H4 时只需在同一 Runner 入口绑定 H4 源码，再沿用既有 FP16 宽行 shape 集合做 Parent/H4 配对运行，无需新增 Runner。

## COMMIT

本地提交，`PUSH=NO`。
