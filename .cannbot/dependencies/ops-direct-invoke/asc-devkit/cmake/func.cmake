# ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------

function(install_mc2_runtime_staging_link src_rel_path dst_rel_path required)
    install(CODE "
set(_src_rel_path \"${src_rel_path}\")
set(_dst_rel_path \"${dst_rel_path}\")
set(_required \"${required}\")
set(_install_prefix \"\${CMAKE_INSTALL_PREFIX}\")
file(TO_CMAKE_PATH \"\${_install_prefix}\" _install_prefix)
set(_src_path \"\${_install_prefix}/\${_src_rel_path}\")
set(_dst_path \"\${_install_prefix}/\${_dst_rel_path}\")
if(EXISTS \"\${_dst_path}\" AND IS_DIRECTORY \"\${_dst_path}\" AND NOT IS_SYMLINK \"\${_dst_path}\")
    message(FATAL_ERROR \"MC2 runtime staging path \${_dst_rel_path} is a directory, cannot create file softlink.\")
elseif(EXISTS \"\${_dst_path}\")
    message(STATUS \"MC2 runtime staging path \${_dst_rel_path} already exists, skip.\")
else()
    if(IS_SYMLINK \"\${_dst_path}\")
        file(REMOVE \"\${_dst_path}\")
    endif()
    if(NOT EXISTS \"\${_src_path}\")
        if(_required)
            message(FATAL_ERROR \"MC2 runtime staging source \${_src_rel_path} does not exist.\")
        else()
            message(WARNING \"MC2 runtime staging source \${_src_rel_path} does not exist, skip creating \${_dst_rel_path}.\")
        endif()
    else()
        get_filename_component(_dst_dir \"\${_dst_path}\" DIRECTORY)
        file(MAKE_DIRECTORY \"\${_dst_dir}\")
        file(RELATIVE_PATH _link_target \"\${_dst_dir}\" \"\${_src_path}\")
        execute_process(
            COMMAND \"${CMAKE_COMMAND}\" -E create_symlink \"\${_link_target}\" \"\${_dst_path}\"
            RESULT_VARIABLE _link_ret
        )
        if(NOT _link_ret EQUAL 0)
            message(FATAL_ERROR \"failed to create MC2 runtime staging link \${_dst_rel_path} -> \${_src_rel_path}.\")
        endif()
        message(STATUS \"Created MC2 runtime staging link \${_dst_rel_path} -> \${_src_rel_path}.\")
    endif()
endif()
" COMPONENT asc-devkit)
endfunction()
