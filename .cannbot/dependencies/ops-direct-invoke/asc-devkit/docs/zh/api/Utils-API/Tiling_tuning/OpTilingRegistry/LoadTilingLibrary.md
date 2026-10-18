# LoadTilingLibrary

## 功能说明

根据输入的路径，加载对应的Tiling动态库。开发者基于工程化算子开发开发方式完成算子实现后，可通过**算子包编译**或**算子动态库编译**获取对应的Tiling动态库文件。

-   算子包编译：Tiling实现对应的动态库为算子包部署目录下的liboptiling.so。具体路径可参考[算子包部署](../../../../guide/programming_guide/advanced_programming/aclnn_operator_development/compilation_and_deployment/basic_process.md#operator-package-deployment)。

-   动态库编译：Tiling实现集成在算子动态库libcust\_opapi.so中。具体路径可参考[算子动态库和静态库编译](../../../../guide/programming_guide/advanced_programming/aclnn_operator_development/compilation_and_deployment/dynamic_static_lib_compilation.md)。

## 函数原型

```
bool LoadTilingLibrary(const char *tilingSoPath) const
```

## 参数说明

| 参数名 | 输入/输出 | 描述 |
| --- | --- | --- |
| tilingSoPath | 输入 | Tiling动态库的路径，支持相对路径与绝对路径。 |

## 返回值说明

true：Tiling动态库加载成功；false：Tiling动态库加载失败。具体错误可参考Log信息。

关于日志配置和查看，请参考[《环境变量参考》](https://www.hiascend.com/document/redirect/CannCommunityEnvRef)中日志相关环境变量。

## 约束说明

无

## 调用示例

```
context_ascendc::OpTilingRegistry tmpIns;
bool flag = tmpIns.LoadTilingLibrary("/your/path/to/so_path/liboptiling.so");
if (flag == false) {
    std::cout << "Load tiling so failed" << std::endl;
    ...
}
// ...
```
