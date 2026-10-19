# R027 GPU 迁移参考

## 路线定位

本目录保存 CUDA、Triton、GPU fused RMSNorm 等实现对 AddRmsNormBias 的可迁移思想。它是研究参考路线，不是可直接上传的 Ascend C 实现。

## 历史来源

- 汇总1：`调研/归档/汇总1/技术路线总报告.md` §4 的 `R014 GPU 迁移参考`、§5 的 `R014`。
- 汇总2：`调研/归档/汇总2/技术路线总报告.md` §5 的 `R023 GPU/编译器迁移与生成参考`。
- 汇总3：`调研/归档/汇总3/技术路线总报告.md` §4 的 GPU/DSL 迁移参考说明。

## 可迁移内容

可提取数据流、行级工作划分、FP32 累加、尾块掩码、epilogue 融合和按 D 分档思路。warp shuffle、atomic、grid sync、动态 shared memory、PTX、GPU mask 假设和 GPU autotune 参数不能直接搬到 Ascend C。

## 提交与实验状态

- 提交资格：仅作为研究参考，不单独提交。
- 当前状态：资料级，未完成真实 CANN/NPU 验证。
- 如形成 Ascend C 实现，必须新建版本目录，并把真正采用的部分映射回 `R001–R029` 的对应实现路线。
