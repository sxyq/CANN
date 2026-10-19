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
- 未新增 Runner 实现、脚本或独立记录表；远端 V005 仅以符号链接复用 V003 的 CMake、Runner wrapper、ABI 与 C++ Runner。

## COMPILE

- `COMPILE=PASS`；Configure 返回码 `0`，Build 返回码 `0`。
- 工具链：cann-server3，CMake 3.22.1，Ascend Toolkit `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`。
- 远端 N01 V005 通过符号链接复用本路线 V003 的 Parent、CMakeLists、Runner wrapper、ABI 和 C++ Runner；H3 include 别名解析到 V005 H4 Candidate。共享 Runner 改为使用 runtime device 1，并移除 `aclrtResetDevice`；事件、流销毁及 `aclFinalize` 仍保留。
- 沿用历史记录中的 `/home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X` 构建区。其 Git 根目录是 `/home/data4t2/lelinfeng/cann` 的 `main` 工作副本，不是单独登记的 Git worktree；远端仅产生 N01/V005 文件，状态为未跟踪，没有远端提交。
- Configure 保留两条原始 Warning：安装路径采用默认值；运行时搜索路径中的 `libstdc++.so.6` 目录可能冲突。Build 最终通过。Compile 未提供寄存器占用数据。
- 产物：`/home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build/paired_runner_n01`。

### Configure

命令：

```bash
ssh -o BatchMode=yes -o ConnectTimeout=8 cann-server3 'export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002; export CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11; cmake -S /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired -B /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002'
```

返回码：`0`。

输出：

```text
CMake Warning at /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/tikcpp/ascendc_kernel_cmake/ASC_CMake/CMakeDetermineASCCompiler.cmake:57 (message):
  CMAKE_INSTALL_PREFIX is not set.  Set to default value
  /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/out.
Call Stack (most recent call first):
  CMakeLists.txt:10 (project)


-- CMAKE_ASC_COMPILER: /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng
-- CMAKE_BUILD_TYPE: Release
-- CMAKE_INSTALL_PREFIX: /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/out
-- ASCEND_CANN_PACKAGE_PATH: /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
-- CMAKE_ASC_LLD_LINKER: /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/tools/ccec_compiler/bin/ld.lld
-- The CXX compiler identification is GNU 11.4.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done
CMake Warning at CMakeLists.txt:28 (add_executable):
  Cannot generate a safe runtime search path for target paired_runner_n01
  because files in some directories may conflict with libraries in implicit
  directories:

    runtime library [libstdc++.so.6] in /usr/lib/aarch64-linux-gnu may be hidden by files in:
      /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/lib64

  Some of these libraries may not be found correctly.


-- Generating done
-- Build files have been written to: /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build
```

### Build

命令：

```bash
ssh -o BatchMode=yes -o ConnectTimeout=8 cann-server3 'export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002; export CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11; cmake --build /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build --parallel 2 --verbose'
```

返回码：`0`。

输出：

```text
/usr/bin/cmake -S/home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired -B/home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build --check-build-system CMakeFiles/Makefile.cmake 0
/usr/bin/cmake -E cmake_progress_start /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build/CMakeFiles /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build//CMakeFiles/progress.marks
/usr/bin/gmake  -f CMakeFiles/Makefile2 all
gmake[1]: Entering directory '/home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build'
/usr/bin/gmake  -f CMakeFiles/paired_runner_n01.dir/build.make CMakeFiles/paired_runner_n01.dir/depend
gmake[2]: Entering directory '/home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build'
cd /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build && /usr/bin/cmake -E cmake_depends "Unix Makefiles" /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build/CMakeFiles/paired_runner_n01.dir/DependInfo.cmake --color=
gmake[2]: Leaving directory '/home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build'
/usr/bin/gmake  -f CMakeFiles/paired_runner_n01.dir/build.make CMakeFiles/paired_runner_n01.dir/build
gmake[2]: Entering directory '/home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build'
[ 50%] Building ASC object CMakeFiles/paired_runner_n01.dir/paired_runner_n01.asc.o
/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng   -I/home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired -I/home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/../parent -I/home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/../candidate -I/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/include -I/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/ascendc/include -I/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/ascendc/include/basic_api/impl -I/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/asc/include/basic_api -I/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/asc/include -I/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/asc -I/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/asc/include/interface -I/usr/include/aarch64-linux-gnu -I/usr/include/c++/11 -I/usr/include/aarch64-linux-gnu/c++/11 -I/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0 -I/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu -I/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward --cce-aicore-arch=dav-c220 -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start -I/usr/include/c++/11 -I/usr/include/aarch64-linux-gnu/c++/11 -I/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0 -I/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu -I/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward -fPIC -O2 -std=c++17 -Xhost-end --npu-arch=dav-2201 -MD -MT CMakeFiles/paired_runner_n01.dir/paired_runner_n01.asc.o -MF CMakeFiles/paired_runner_n01.dir/paired_runner_n01.asc.o.d -o CMakeFiles/paired_runner_n01.dir/paired_runner_n01.asc.o -c -x asc /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/paired_runner_n01.asc
[100%] Linking ASC executable paired_runner_n01
/usr/bin/cmake -E cmake_link_script CMakeFiles/paired_runner_n01.dir/link.txt --verbose=1
/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/ccec_compiler/bin/bisheng CMakeFiles/paired_runner_n01.dir/paired_runner_n01.asc.o -o paired_runner_n01   -L/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64  -L/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/lib64  -L/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/lib64  -L/usr/lib/gcc/aarch64-linux-gnu/11  -Wl,-rpath,/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64 -Wl,-rpath,/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/lib64 -Wl,-rpath,/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/lib64 -Wl,-rpath,/usr/lib/gcc/aarch64-linux-gnu/11 -ltiling_api -lregister -lplatform -lunified_dlog /usr/lib/aarch64-linux-gnu/libstdc++.so.6 -ldl -lm -lgraph_base
gmake[2]: Leaving directory '/home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build'
[100%] Built target paired_runner_n01
gmake[1]: Leaving directory '/home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build'
/usr/bin/cmake -E cmake_progress_start /home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build/CMakeFiles 0
```

## CORRECTNESS / LOCAL

- 本节依据既有 V005 输出和快照记录；本次更新没有运行 NPU 命令，也没有生成新样本。
- Candidate 源码为 V005 H4。复用的 `paired_runner_n01.asc` 仍以 `N01-ROW-CORE-GEOMETRY-H3_kernel.asc` 名称包含 Candidate，因此 Runner 输出中的 `binary=N01-H3` 指向 V005 H4，不代表运行了 V003 Candidate。
- 环境记录：`correctness-v005-npu1-20261016T140830Z.log` 保留了错误路径 `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/set_env.sh` 不存在的原始输出；该次返回码 `1`，Runner 未启动。后续成功输出使用通用入口 `/usr/local/Ascend/ascend-toolkit/set_env.sh`，并在其后将 `/usr/lib/aarch64-linux-gnu` 前置到 `LD_LIBRARY_PATH`。指定的 `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh` 没有用于下述既有运行，本次也未尝试。
- Parent 资格运行：日志时间 `2026-10-16T14:11:16Z`，模式 `--qualify-v011`，返回码 `0`；18/18 Parent Correctness=PASS。原始输出：`correctness-v005-npu1-corrected-env-20261016T141116Z.log`。
- 配对 Local 运行：日志时间 `2026-10-16T14:13:20Z`，模式 `--paired`，返回码 `0`；18 个用例各有 21 对交错采样，共 378 对。18/18 Parent 与 18/18 V005 H4 Candidate Correctness=PASS，最终输出 `RUNNER_COMPLETE mode=PAIRED correctness=PASS cases=18`。原始逐样本输出：`local-v005-npu1-corrected-env-20261016T141320Z.log`。Runner 在每个用例采完 21 对后才输出该用例的正确性结果；因此 Candidate Correctness 与 Local 来自同一次配对运行，不能记成 Candidate Correctness 先于该用例的 Local。

设备事件中位数及配对差值（H4 Candidate - V011，单位 us；正值表示 Candidate 较慢）：

| 用例 | Parent | H4 Candidate | 配对差值中位数 |
|---|---:|---:|---:|
| fp16-tail-d12288-r2 | 6.000 | 7.560 | +0.200 |
| fp16-tail-d12288-r8 | 16.600 | 6.820 | -0.640 |
| fp16-wide-d32768-r1 | 10.040 | 16.980 | +2.100 |
| fp16-wide-d32768-r2 | 20.540 | 16.800 | -2.900 |
| fp16-wide-d32768-r3 | 17.820 | 17.560 | +0.380 |
| fp16-wide-d32768-r4 | 21.100 | 17.260 | -4.600 |
| fp16-wide-d32768-r6 | 17.700 | 21.780 | +3.960 |
| fp16-wide-d32768-r8 | 18.520 | 19.640 | +1.760 |
| fp16-wide-d32768-r12 | 31.180 | 30.980 | -0.240 |
| bf16-tail-d12288-r2 | 17.880 | 17.700 | +0.640 |
| bf16-tail-d12288-r8 | 15.880 | 14.840 | +0.200 |
| bf16-wide-d32768-r1 | 14.220 | 19.480 | +0.660 |
| bf16-wide-d32768-r2 | 21.180 | 24.540 | -0.020 |
| bf16-wide-d32768-r3 | 18.620 | 14.300 | -2.500 |
| bf16-wide-d32768-r4 | 14.220 | 14.420 | +0.240 |
| bf16-wide-d32768-r6 | 14.560 | 14.740 | +0.140 |
| bf16-wide-d32768-r8 | 26.520 | 27.260 | +1.600 |
| bf16-wide-d32768-r12 | 25.320 | 26.060 | +0.820 |

18 个用例的配对差值中位数有 12 个为正、6 个为负，其算术平均为 `+0.100 us`。`LOCAL_SCORE=NONE`：当前结果没有形成稳定的整体性能收益证据。

- NPU1 运行前快照：`npu1-preflight-correctness-20261016T141116Z.txt` 与 `npu1-preflight-local-20261016T141320Z.txt`；两次均记录 Health=`OK`、HBM=`3480/65536 MiB`、AICore=`0%`、无进程。
- Postflight：`2026-10-16T14:15:30Z`，见 `npu1-postflight-local-20261016T141530Z.txt`；NPU1 Health=`OK`、HBM=`3480/65536 MiB`、AICore=`0%`、无进程。
- 分配记录留空：Main 登记的新 lease 时间为 `14:16:49Z`，晚于 Local 与 postflight 文件时间；本结果不把该 lease 记作这次运行的设备分配，Main 将按实际时间更新设备记录。
- 更早的采样记录原样保留：`npu1-preflight-20261016T134503Z.txt` 中 `npu-smi info -i 1` 返回 `215` 并提示需指定 `-t type`；同一时段的 usages、health、proc-mem 查询均返回 `0`。完整快照 `npu1-preflight-full-20261016T135523Z.txt` 使用 `npu-smi info`，返回 `0`；`npu1-preflight-run-20261016T140719Z.txt` 也保留为后续失败环境尝试前的快照。此前采样文件未覆盖，`V003/parent/` 数据保持原样。

## NEXT DEVICE ENTRY

本次只整理既有记录，没有新增设备验证。正确性与 Local 现有输出均完成；`LOCAL_SCORE=NONE`。指定 alpha002 `aarch64-linux/script/set_env.sh` 路径尚无运行记录，本次未作设备重跑。

## COMMIT

候选源码提交：`33494f43`；Compile 记录提交：`7b0a5262`。本地仅提交 N01 Runner 修改和 V005 结果/原始记录，`PUSH=NO`。
