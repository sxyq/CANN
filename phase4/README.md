# Phase4

这是新的 Clean-Room 探索区域。

禁止读取：

`../归档/`

Phase4 第一轮禁止使用 Git 历史恢复旧实现，包括：

```text
git log
git show
git reflog
git branch -a
git tag
git worktree list
git diff <historical-ref>
git show <historical-ref>:<path>
```

禁止访问任何非当前 Phase4 文件的旧 commit、branch 或 tag。

允许的 Git 操作仅限：

```text
git status
git add
git commit
```

以及 Phase4 新产生 commit 的必要操作。

不得通过 GitHub remote branches 搜索历史 kernel。

所有技术规划由下一次全新会话完成。
