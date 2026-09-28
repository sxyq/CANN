cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)
project(addrmsnormbias LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(ACL_INCLUDE_DIR "$ENV{ASCEND_HOME_PATH}/aarch64-linux/include")
set(ACL_LIB_DIR "$ENV{ASCEND_HOME_PATH}/lib64")

add_executable(addrmsnormbias op_host/addrmsnormbias_host.asc)

target_include_directories(addrmsnormbias PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/op_kernel
    ${CMAKE_CURRENT_SOURCE_DIR}/op_host
    ${ACL_INCLUDE_DIR}
)

target_link_directories(addrmsnormbias PRIVATE
    ${ACL_LIB_DIR}
    "$ENV{ASCEND_HOME_PATH}/aarch64-linux/lib64"
)

target_link_libraries(addrmsnormbias PRIVATE
    tiling_api
    register
    platform
    unified_dlog
    dl
    m
    graph_base
    ascendcl
    ascendc_runtime
)

target_compile_options(addrmsnormbias PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
)
