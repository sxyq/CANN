# CANNJudge 提交要求

线上提交由 Main 执行，Child 不执行线上提交。

入口：

```text
/Users/sunyiyang/Desktop/Project/cann/脚本/cannjudge-submit.mjs
```

提交前必须满足：

- Compile PASS；
- source 文件包含合法 Kernel 入口；
- source 路径位于对应 Phase4 workspace；
- Main 已形成对应 Phase4 commit。

Main 使用：

```text
npm run cannjudge:submit -- --yes --source /absolute/path/to/source
```

结果保存到对应 `phase4/online/<candidate>/`，并写入 `phase4/control/results.tsv`。

results.tsv 字段：

```text
candidate	architecture	agent	workspace	commit	compile	submission_id	online_status	pass_count	official_score	hot_path_domain	fallback_domain	note
```

Child 在六个候选全部 terminal 前不得获知任何分数、排序或其它候选结果。

