# ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2025 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------

set(ASC_CMAKE_MODULE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/ascendc_kernel_cmake)
add_custom_target(ascendc_kernel_cmake ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ASC_CMAKE_MODULE_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/asc" "${ASC_CMAKE_MODULE_BINARY_DIR}"
)

install(DIRECTORY ${ASC_CMAKE_MODULE_BINARY_DIR}
    DESTINATION ${INSTALL_LIBRARY_DIR}/tikcpp ${INSTALL_OPTIONAL} COMPONENT asc-devkit
    FILE_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
    PATTERN "test_*" EXCLUDE
    PATTERN "aicpu-config.cmake" EXCLUDE
    PATTERN "asc-config.cmake" EXCLUDE
)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/asc/aicpu-config.cmake
    DESTINATION ${INSTALL_LIBRARY_DIR}/lib64/cmake ${INSTALL_OPTIONAL} COMPONENT asc-devkit
)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/asc/asc-config.cmake
    DESTINATION ${INSTALL_LIBRARY_DIR}/lib64/cmake ${INSTALL_OPTIONAL} COMPONENT asc-devkit
)
