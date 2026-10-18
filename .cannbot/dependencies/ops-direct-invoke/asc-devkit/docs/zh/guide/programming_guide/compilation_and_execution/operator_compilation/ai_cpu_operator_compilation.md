# AI CPU算子编译

与AI Core算子只需一个`.asc`文件即可编译生成可执行文件不同，AI CPU算子需要同时包含`.aicpu`文件（核函数（Kernel）定义）和`.asc`文件（通过内核调用符调用核函数（Kernel））才能编译生成可执行文件。

## 通过bisheng命令行编译

下文基于一个Hello World打印样例来讲解如何通过bisheng命令行编译AI CPU算子。该样例包含`hello_world.aicpu`文件（AI CPU核函数（Kernel）定义）和`main.asc`文件（通过内核调用符调用AI CPU核函数（Kernel））。

hello\_world.aicpu文件内容如下：

```
#include "aicpu_api.h"

__global__ __aicpu__ uint32_t hello_world(void *args)
{
    AscendC::printf("Hello World!!!\n");
    return 0;
}
```

Host侧使用内核调用符<<<...\>\>\>进行AI CPU算子的调用， main.asc示例代码如下：

```
#include "acl/acl.h"

struct KernelArgs {
    int mode;
};

extern __global__ __aicpu__ uint32_t hello_world(void *args);

int32_t main(int argc, char const *argv[])
{
    aclInit(nullptr);
    int32_t deviceId = 0;
    aclrtSetDevice(deviceId);
    aclrtStream stream = nullptr;
    aclrtCreateStream(&stream);

    struct KernelArgs args = {0};
    constexpr uint32_t numBlocks = 1;
    hello_world<<<numBlocks, nullptr, stream>>>(&args, sizeof(KernelArgs));
    aclrtSynchronizeStream(stream);

    aclrtDestroyStream(stream);
    aclrtResetDevice(deviceId);
    aclFinalize();
    return 0;
}
```

开发者可以使用bisheng命令行将hello\_world.aicpu与main.asc分别编译成.o，再链接成为可执行文件，编译命令如下：

-   编译hello\_world.aicpu时，通过-I指定依赖头文件所在路径；通过--cce-aicpu-laicpu\_api为Device链接依赖的库libaicpu\_api.a，通过--cce-aicpu-L指定libaicpu\_api.a的库路径；通过--cce-aicpu-toolkit-path指定AI CPU编译工具链路径；通过--cce-aicpu-sysroot指定AI CPU编译所使用的系统根目录。
-   编译main.asc时，通过--npu-arch编译选项指定对应的架构版本号。

`${INSTALL_DIR}`请替换为CANN软件安装后文件存储路径。以root用户安装为例，安装后文件默认存储路径为：/usr/local/Ascend/cann。

各产品型号对应的架构版本号请通过[对应关系表](../../language_extension/simd_builtin_keywords.md#npu-arch)进行查询。

```
bisheng -O2 hello_world.aicpu --cce-aicpu-L${INSTALL_DIR}/lib64/device/lib64 --cce-aicpu-laicpu_api --cce-aicpu-toolkit-path=${INSTALL_DIR}/toolkit/toolchain/hcc/bin --cce-aicpu-sysroot=${INSTALL_DIR}/toolkit/toolchain/hcc/sysroot -I${INSTALL_DIR}/asc/include/aicpu_api -c -o hello_world.aicpu.o
bisheng --npu-arch=dav-2201 main.asc -c -o main.asc.o
bisheng hello_world.aicpu.o main.asc.o -o demo
```

上文我们通过一个入门示例介绍了使用bisheng命令行编译生成可执行文件的示例。除此之外，使用bisheng命令行也支持编译生成AI CPU算子的动态库与静态库，用户可在asc代码中通过内核调用符<<<...\>\>\>调用AI CPU算子的核函数（Kernel），并在编译asc代码源文件生成可执行文件的时候，链接AI CPU动态库或者静态库，注意：若单独编译AI CPU算子代码生成动态库、静态库时，需要手动链接[表格中的库文件](ai_core_operator_compilation.md#内置链接库)。

-   编译生成算子动态库

    ```
    # 编译test_aicpu.cpp生成算子动态库
    # -lxxx表示默认链接库
    # bisheng -shared -x aicpu test_aicpu.cpp -o libtest_aicpu.so -lxxx ...
    ```

-   编译生成算子静态库

    ```
    # 编译test_aicpu.cpp生成算子静态库
    # -lxxx表示默认链接库
    # bisheng -lib -x aicpu test_aicpu.cpp -o libtest_aicpu.a -lxxx ...
    ```

## AI CPU算子常用编译选项

AI CPU算子常用的编译选项说明如下：

| 选项 | 是否必需 | 说明 |
| --- | --- | --- |
| -help | 否 | 查看帮助。 |
| -x | 否 | 指定编译语言。
指定为aicpu时表示AI CPU算子编程语言。 |
| -o <file> | 否 | 指定输出文件的名称和位置。 |
| -c | 否 | 编译生成目标文件。 |
| -shared，--shared | 否 | 编译生成动态链接库。 |
| -lib | 否 | 编译生成静态链接库。 |
| -g | 否 | 编译时增加调试信息。 |
| -fPIC | 否 | 告知编译器产生位置无关代码。 |
| -O | 否 | 用于指定编译器的优化级别，当前支持-O3，-O2，-O0。 |
| --cce-aicpu-L | 否 | 指定AI CPU Device依赖的库路径。 |
| --cce-aicpu-l | 否 | 指定AI CPU Device依赖的库。 |

## CMake方式编译

项目中可以使用CMake来更简便地使用毕昇编译器编译AI CPU算子，生成可执行文件、动态库、静态库或二进制文件。

仍以[通过bisheng命令行编译](#section153291123460)中介绍的Hello World打印样例为例，除了代码实现文件，还需要在工程目录下准备一个CMakeLists.txt。

```
├── hello_world.aicpu // AI CPU算子核函数（Kernel）定义
├── main.asc // AI CPU算子核函数（Kernel）调用
└── CMakeLists.txt
```

CMakeLists.txt内容如下：

```
cmake_minimum_required(VERSION 3.16)
# 1、find_package()是CMake中用于查找和配置Ascend C编译工具链的命令
find_package(ASC REQUIRED)
find_package(AICPU REQUIRED)

# 2、指定项目支持的语言包括ASC、AICPU和CXX，ASC表示支持使用毕昇编译器对Ascend C编程语言进行编译，AI CPU表示支持使用毕昇编译器对AI CPU算子进行编译
project(kernel_samples LANGUAGES ASC AICPU CXX)

# 3、使用CMake接口编译可执行文件
add_executable(demo
    hello_world.aicpu
    main.asc
)

#4、由于存在ASC与AI CPU语言，需要指定链接器
set_target_properties(demo PROPERTIES LINKER_LANGUAGE ASC)  # 指定链接使用语言

target_compile_options(demo PRIVATE
    # --npu-arch用于指定NPU的架构版本，dav-后为架构版本号
    # <COMPILE_LANGUAGE:ASC>:表明该编译选项仅对语言ASC生效
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
)
```

各产品型号对应的架构版本号请通过[对应关系表](../../language_extension/simd_builtin_keywords.md#npu-arch)进行查询。

如果需要CMake编译编译生成动态库、静态库，下面提供了更详细具体的编译示例：

-   编译.cpp文件生成动态库

    ```
    # 将.cpp文件置为ASC属性，启用Ascend C语言进行编译
    set_source_files_properties(
        add_custom_base.cpp
        sub_custom_base.cpp
        PROPERTIES LANGUAGE ASC
    )

    # 将.cpp文件置为AICPU属性，支持AI CPU算子编译
    set_source_files_properties(
        aicpu_kernel.cpp
        PROPERTIES LANGUAGE AICPU
    )

    add_library(kernel_lib SHARED
        add_custom_base.cpp
        sub_custom_base.cpp
        aicpu_kernel.cpp # 支持AI CPU算子与AI Core算子一起打包为动态库
    )

    target_compile_options(kernel_lib PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
    )

    # AI CPU算子编译时，需要手动链接以下依赖库（若指定链接语言为ASC时，不需要手动链接以下库）
    target_link_libraries(kernel_lib PRIVATE
        ascendc_runtime
        profapi
        unified_dlog
        ascendcl
        runtime
        c_sec
        mmpa
        error_manager
        ascend_dump
    )

    add_executable(demo
        main.asc
    )
    target_link_libraries(demo PRIVATE
        kernel_lib
    )
    ```

-   编译.asc文件与.aicpu文件生成静态库

    ```
    # .asc文件会默认启用Ascend C语言进行编译，.aicpu文件会默认启用AICPU语言进行编译，不需要通过set_source_files_properties进行设置
    add_library(kernel_lib STATIC
        add_custom_base.asc
        sub_custom_base.asc
        aicpu_kernel.aicpu  # 可支持AI CPU算子与AI Core算子一起打包为静态库
    )

    target_compile_options(kernel_lib PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
    )

    add_executable(demo
        main.asc
    )
    target_link_libraries(demo PRIVATE
        kernel_lib
    )
    ```
