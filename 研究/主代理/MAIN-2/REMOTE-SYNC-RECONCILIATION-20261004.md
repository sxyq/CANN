# MAIN-2 Remote Sync Reconciliation — 2026-10-04

At the last successful `git fetch origin --prune`, the five non-UB exact route
targets were absent. Their local upstreams had incorrectly pointed at
`origin/main`; those branch configs are now set to their exact route target
refs. Current `git branch -vv` shows the five target refs as `gone`, not synced.

| Route | Worktree | Local branch / SHA | Upstream / target remote | Remote SHA | Relation | Status |
|---|---|---|---|---|---|---|
| ADDR | `/home/data4t2/lelinfeng/cann/worktrees/w2/m2/hotloop-addr` | `w2/m2/hotloop-addr` / `fdaf5b3815f12484d0c8b7341782746f1a669c72` | `origin/w2/m2/hotloop-addr` | No tracking ref at last fetch; live SHA unverified | Target absent at last fetch | `PUSH_PENDING_AUTH` |
| BRANCH | `/home/data4t2/lelinfeng/cann/worktrees/w2/m2/hotloop-branch` | `w2/m2/hotloop-branch` / `70735a73605f3cabb41a6c0953ee0d05a1bef592` | `origin/w2/m2/hotloop-branch` | No tracking ref at last fetch; live SHA unverified | Target absent at last fetch | `PUSH_PENDING_AUTH` |
| UB | `/home/data4t2/lelinfeng/cann-w2-m2-ub` | `w2/m2/ub-lifetime-safe` / `b34f94b2c214574c0237f1b677b066b484f98ec7` | `origin/w2/m2/ub-lifetime-safe` | Last fetched `a9c9affcb49e8591f803ab409aafa6192a34caca`; live SHA unverified | Merge-base `ed860e392d7604694ac6664da60aff1fc1f4c04f`; local +28 / remote +2 | `DIVERGED_REMOTE / UB_SYNC_BLOCKED_NEEDS_PLANNING` |
| REDUCE | `/home/data4t2/lelinfeng/cann/worktrees/w2/m2/reduce-finalize` | `w2/m2/reduce-finalize` / `52bd18a86872bdbc4f1d79b71ed65240e9977234` | `origin/w2/m2/reduce-finalize` | No tracking ref at last fetch; live SHA unverified | Target absent at last fetch | `PUSH_PENDING_AUTH` |
| TILECOUNT | `/home/data4t2/lelinfeng/cann/worktrees/w2/m2/tilecount-unroll` | `w2/m2/tilecount-unroll` / `38ff22e593730f041e134357c6087bcce981c229` | `origin/w2/m2/tilecount-unroll` | No tracking ref at last fetch; live SHA unverified | Target absent at last fetch | `PUSH_PENDING_AUTH` |
| MAIN-2 control | `/home/data4t2/lelinfeng/cann-w2-main2-control` | `w2/main2/control` / `d307b91e2b8cc78a8bcb39d447c911918807c3d4` | `origin/w2/main2/control` | No tracking ref at last fetch; live SHA unverified | Target absent at last fetch | `PUSH_PENDING_AUTH` |

## Bounded push attempts

- First normal-push window: explicit exact-branch refspecs, `timeout 60`; all
  five missing-target branches ended with RC 124 and no successful push
  output. UB was not pushed because its graph was already known to diverge.
- Second normal-push window: same exact refspecs and no force options. ADDR,
  BRANCH, REDUCE, TILECOUNT, and MAIN-2 control each returned RC 128:
  `fatal: could not read Username for 'https://github.com': No such device
  or address`.
- A read-only SSH `ls-remote` check returned `Host key verification failed`.
  SSH known-host trust/configuration was not changed. No remote SHA equality
  is claimed; no push succeeded.
- UB's remote-only commits `85b95db8` and `a9c9affc` carry evidence that must
  be preserved. No merge/rebase/push was attempted; see
  `UB_REMOTE_RECONCILIATION.md`.

```text
PUSHED_HEAD=NONE
PUSH_VERIFIED=NO
FORCE_PUSH=NO
RESET/REBASE/CLEAN=NO
UPSTREAMS_CORRECTED=YES (exact target refs configured)
```

All local route evidence and untracked build outputs were preserved. The
canonical `main` worktree was not modified; its existing untracked user data
remains untouched. Remote write synchronization remains blocked on usable
GitHub authentication / verified SSH host trust.
