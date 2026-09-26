# Main-1 ASYNC-TRIPLE-X collision copy (V001)

`V001/` holds the contents of `phase4/local/ASYNC-TRIPLE-X/V001/` and
`V001-workspaces/` holds the contents of `phase4/workspaces/ASYNC-TRIPLE-X/V001/`
as they existed in the Main-1 first-round worktree
`/Users/sunyiyang/Desktop/Project/cann-sixlane/ASYNC-TRIPLE-X`.

- branch: `exec/sixlane-20260924-async-triple-x`
- commit: `16325183730c49b5b5896833c98eb55202ca6a0f` (`phase4: retain async route collision evidence`)
- candidate SHA256: `61223a486cca4c54e099f760e9f48a4cd2fbedb2645967b936fe80d2785e1e1a`
- status: `COLLISION_FROZEN` / `HISTORICAL_ONLY` — parent and Candidate both returned
  `aclrtSynchronizeStream` 507035, correctness was never assessed, no timing, no Online.

Why it is not at `phase4/local/ASYNC-TRIPLE-X/V001/`: that path belongs to the
Main-2 route instance (`exp/next6-async-triple-x`), whose V001 is a different
candidate (SHA `2defc6c2…`, correctness PASS, measurement blocked). The two
records collide at the revision path, so the Main-1 copy is retained here
instead of being merged into the Main-2 record.

See `phase4/local/ASYNC-TRIPLE-X/V001/COLLISION-HANDOFF.md` inside this directory,
`phase4/control/main1-route-selection.md`, and `phase4/control/main1-revision-ledger.tsv`.
