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
- 远端 N01 V005 复用 V003 的 Parent、CMakeLists、Runner wrapper、ABI 和 C++ Runner 符号链接；H3 include 别名解析到 V005 H4 源码。生成的 include 目录指向 V005，V003 源文件未改。
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

- `CORRECTNESS=NOT_RUN`；`LOCAL_SCORE=NONE`。
- 本轮没有执行任何 NPU 正确性或性能运行，符合任务范围。

## NEXT DEVICE ENTRY

需要设备验证：Compile 只确认构建通过，尚未确认数值正确性或性能。后续沿用 `V003/paired/paired_runner.cpp` 与 `paired/CMakeLists.txt`，使用本轮生成的 `/home/data4t2/lelinfeng/cann/server_runs/N01-ROW-CORE-GEOMETRY-X/V005/paired/build/paired_runner_n01`，先跑既有 Parent/H4 Correctness 用例，再按既有 FP16 宽行 shape 集合做配对 Local；本轮未启动 Runner。

## COMMIT

候选源码提交：`33494f43`；本轮 Compile 记录只提交 V005 的 `RESULT.md`，`PUSH=NO`。
