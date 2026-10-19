cmake_minimum_required(VERSION 3.16)
find_package(ASC REQUIRED)
project(k_R31A__V018 LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 17)
ascendc_library(k SHARED "/home/data4t2/lelinfeng/server_full_audit/src/R31A__V018.asc")
target_compile_options(k PRIVATE $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_compile_options(k PRIVATE $<$<COMPILE_LANGUAGE:ASC>:--npu-soc=Ascend910B3>)
target_compile_options(k PRIVATE $<$<COMPILE_LANGUAGE:ASC>:-include> $<$<COMPILE_LANGUAGE:ASC>:/home/data4t2/lelinfeng/server_full_audit/judge_abi.h>)
target_include_directories(k PRIVATE "/home/data4t2/lelinfeng/server_full_audit/src" "/home/data4t2/lelinfeng/server_full_audit/inc" "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/include" "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/include" "/home/data4t2/lelinfeng/server_full_audit/src")
target_link_directories(k PRIVATE "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64" "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64")
target_link_libraries(k PRIVATE ascendcl)
target_link_directories(k PRIVATE
  /usr/lib/gcc/aarch64-linux-gnu/11
  /usr/lib/aarch64-linux-gnu
  /lib/aarch64-linux-gnu
  "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/lib64")
target_link_options(k PRIVATE -Wl,-rpath-link,/usr/lib/gcc/aarch64-linux-gnu/11)
