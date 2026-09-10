# AddRmsNormBias 源码清单
#
# CANN 算子工程包含内部 CMake 宏（npu_op_code_gen 等），
# 不同版本模板差异较大。**强烈建议真机环境下先用 msopgen 生成骨架**：
#
#   msopgen gen -i add_rms_norm_bias.json -c ai_core-<soc> -lang cpp -out <dir>
#
# 然后把本目录 op_host/、op_kernel/ 中的实现文件复制/覆盖到生成骨架对应目录，
# 使用生成骨架自带的 CMakeLists.txt 与 build.sh 编译。本文件不承担 CANN 编译。

cmake_minimum_required(VERSION 3.16)
project(AddRmsNormBiasSourceManifest LANGUAGES NONE)

add_custom_target(source_manifest ALL
    COMMAND ${CMAKE_COMMAND} -E echo "AddRmsNormBias source files are ready for an msopgen project"
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/source_manifest.stamp
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/source_manifest.stamp
)
