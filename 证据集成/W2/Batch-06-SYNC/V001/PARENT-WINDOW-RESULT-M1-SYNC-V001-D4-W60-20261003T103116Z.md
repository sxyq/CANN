# SYNC-TOPOLOGY-CHAMPION-X V001 Parent Window Result

## Run identity

- Lease ID: `M1-SYNC-V001-D4-PARENT-WINDOW-W60-20261003T103116Z`
- RUN_ID: `M1-SYNC-V001-D4-PARENT-WINDOW-W60-20261003T103116Z-RETRY2-20261003T105056Z`
- Device and probe: d4, `[2,12288]` FP16
- Direct Parent: R31B-V011, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate source SHA256: `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`
- Timing executable SHA256: `99236e9e0e36c6c539cfc102bf149e84ba20a89b83bddf441cb00ba9bd9d4413`
- Candidate and Parent module SHA256: `1a7172946ceab8786920f0e386d2df11307d58e22d86a8f6da90d11cce3844f4` and `017ab02a4fe0da2c2692ba18c17c89ad27cd928ef66a14bf8035d2c4731ec4ec`
- Runner source: commit `041e653f`; `runner_main.inc` SHA256 `31f4d25b314f7a965d7bc00cc1b2625a761b6c752ad0fd4c1956a29cacd34c4a`

## Execution

Only Parent PRECHECK-A and PRECHECK-B ran. Each block used six fresh processes; each process used 60 synchronized warmups followed by 21 device-event samples. All 12 rows in the remote `runner-status.tsv` have RC=0, `RUNNER_COMPLETE ... PASS`, and Parent correctness PASS (`matched_ratio=1.0`, `max_abs=0.00048828125`). The 12 raw files each contain 21 samples.

The raw samples, runner logs, status table, and resource snapshots remain on server3 at:

`/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/M1-SYNC-V001-D4-PARENT-WINDOW-W60-20261003T103116Z-RETRY2-20261003T105056Z/`

The first summary invocation used server Node 12.22.9 and stopped with `SyntaxError: Unexpected reserved word`; it left `window-qualification.json` empty. A later Node 12 run also established that this runtime does not resolve `node:fs/promises`. Commit `c3a7719f` wraps the existing asynchronous summary flow in `main()` and uses `fs.promises` with the `fs` and `path` built-ins. No sample, threshold, or statistic changed. The updated file produced the same summary under server Node 12.22.9 and workstation Node 24.21.0.

## Qualification

Both blocks pass the existing limits: CV at most `0.15` and max/min at most `1.30`.

| Block | Six process medians (us) | Median (us) | CV | Max/min | Result |
|---|---|---:|---:|---:|---|
| PRECHECK-A | 8.22, 8.42, 8.46, 8.46, 8.42, 8.40 | 8.42 | 0.010696 | 1.029197 | PASS |
| PRECHECK-B | 8.58, 8.62, 8.36, 8.34, 8.50, 8.28 | 8.43 | 0.016518 | 1.041063 | PASS |

`PARENT_WINDOW_QUALIFICATION=QUALIFIED`. The remote run's `final-status.txt` says `WINDOW_UNQUALIFIED` because the previous Node 12.22.9 summarizer used `await` inside a non-async `try` block at line 71 and stopped with `SyntaxError: Unexpected reserved word`. Its wrapper treated that summary-tool exit as a failed window. Recalculation from the unchanged raw files qualifies both blocks. The Node 12-compatible summarizer in commit `c3a7719f` produced the same result under Node 12.22.9 and workstation Node 24.21.0; no sample or statistic changed.

No same-binary mode or Parent/Candidate pair ran under this lease. In particular, the prior same-binary PASS used executable SHA256 `7815e6b6c49f1662b245c10e24ee6f0eca8724d455e81b8755c29d331494c6ca`; it is not evidence for this window's timing executable SHA256 `99236e9e0e36c6c539cfc102bf149e84ba20a89b83bddf441cb00ba9bd9d4413`. Same-binary for the latter executable remains NOT_RUN. Candidate P/C timing was NOT_RUN and no Local performance verdict is claimed.

The window ended and its lease was released at `2026-10-03T10:52:23Z`. The canonical lease ledger contains the RELEASED entry recorded by Support-B; this route worktree did not alter the shared lease table. No further device work is included in this result.

## Resource snapshots

- At `2026-10-03T10:50:56Z`: d4 HBM `59190/65536 MB` (6346 MB free), AICore `0%`; project disk available `592 GB`.
- After the window at `2026-10-03T10:52:23Z`: d4 HBM `59192/65536 MB`, AICore `0%`; project disk available `592 GB`.
- VLLM PID `2999855` remained present. No SYNC runner or build process remained after completion.
