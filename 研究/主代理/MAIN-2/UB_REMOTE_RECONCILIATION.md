# UB Route Remote Reconciliation — 2026-10-04

```text
LOCAL_BRANCH=w2/m2/ub-lifetime-safe
LOCAL_HEAD=b34f94b2c214574c0237f1b677b066b484f98ec7
TARGET_REMOTE_BRANCH=origin/w2/m2/ub-lifetime-safe
LAST_FETCHED_REMOTE_HEAD=a9c9affcb49e8591f803ab409aafa6192a34caca
MERGE_BASE=ed860e392d7604694ac6664da60aff1fc1f4c04f
LOCAL_ONLY_COMMITS=28
REMOTE_ONLY_COMMITS=2
LIVE_REMOTE_QUERY=NETWORK_TIMEOUT
SYNC_STATUS=UB_SYNC_BLOCKED_NEEDS_PLANNING
```

## Remote-only commits that must be preserved

`git log --left-right local...origin/w2/m2/ub-lifetime-safe` showed two
remote-only commits:

- `85b95db89a35b048a9386f7400d97e904f402d88` — adds the remote
  `研究/UB-LIFETIME-SAFE-CHAMPION-X/TRACK-B-HANDOFF.md` evidence audit.
- `a9c9affcb49e8591f803ab409aafa6192a34caca` — adds a further UB lifetime
  evidence-gap note to that handoff.

The local side contains 28 commits not in the fetched remote tip, including
V001 correctness/forensic/score evidence. A tip-to-tip read-only diff also
shows the route evidence trees have since changed: the remote version lacks
local forensic/handoff files and its handoff content differs. This is not a
fast-forward-compatible push, and replacing either side would risk losing
evidence.

## Action and boundary

- The local upstream was corrected to `origin/w2/m2/ub-lifetime-safe`; it now
  reports `ahead 28, behind 2` relative to the proper target.
- No push was attempted because normal push cannot fast-forward this graph.
- No merge, rebase, reset, cleanup, or history rewrite was performed. Local
  untracked build outputs remain untouched.
- Preserve both commit sets. A Planning/sync owner must review a merge-based
  reconciliation and resolve the changed UB evidence tree before a normal
  push can be attempted.

```text
FORCE_PUSH=FORBIDDEN
LOCAL_EVIDENCE_PRESERVED=YES
UB_SYNC_BLOCKED_NEEDS_PLANNING=YES
```
