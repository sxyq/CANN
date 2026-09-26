# HANDOFF — SCHED-ROWGROUP-X V001

Status: **ONLINE_CANDIDATE**
Date (UTC): 2026-09-25
Route: SCHED-ROWGROUP-X | Worktree: cann-next6/SCHED-ROWGROUP-X | Branch: exp/next6-sched-rowgroup-x
Device: cann-server3 (hwnput3) ASCEND_DEVICE_ID=4 (d5/d6 untouched, d7 not used)
Lease: LH3-SCHED-33x100 (local record only; shared lease TSV not edited per route brief)

## SHA

- V001 `submission.asc` SHA256 **unchanged**: `0fae0a42e3942356fe3477cd518b17180b6104d57350cee705cba37a5895e65c`
  (verified local before run and on server include path after run).
- Direct Parent SHA: `62de32dfc56a2b258704f658115fd01c8b224c6b814c106c049fd8cc9070c216` (parent score 17.14).
- No kernel, runner, or harness edits this cycle.

## Track-A result (one clean attempt, unified DEVICE_EVENT protocol)

1. Same-binary exact-shape 33×100 FP32 (Parent vs itself): **PASS** — ALL MAD/med 0.0536,
   block drift 0.0990 (both ≤0.10). 1 attempt, no retries used.
2. Same-binary exact-shape 17×256 FP32: **PASS** — MAD/med 0.0262, drift 0.0247.
3. Interleaved P/C (PC/CP/PC/CP, 31 samples/process, same window):
   - 33×100: deltas −51.68 / −51.07 / −52.04 / −50.99 % → **median −51.38%, 4/4 favor V001,
     no reverse pair, order-robust**.
   - 17×256 (aligned control): −25.80 (outlier-tailed P block) / −4.16 / +2.03 / −0.20 %
     → median −2.18%, ex-p1 ~−0.2% → **in noise floor, control neutral as expected**.
4. Correctness: `bad=0` on all 18 runs. Load: AICore 0% on d4, VLLMEngineCor HBM-resident
   (documented residual); robust stats within registered thresholds.

Prior blockers resolved: same-binary drift (was 0.1999/0.1036) now 0.0990 PASS; reverse pair
on 33×100 gone (4/4); C within-block MAD now 0.015–0.034.

## Evidence paths

- `phase4/local/SCHED-ROWGROUP-X/V001/support/results-lh3-sb33/` (33×100 same-binary)
- `phase4/local/SCHED-ROWGROUP-X/V001/support/results-lh3-pc/SUMMARY.md` + raw/stats (P/C, 17×256 same-binary)
- Server originals: `/home/data4t2/lelinfeng/phase4-workspaces/SCHED-ROWGROUP-X/support/results-lh3-{sb33,pc}/`

## Standing constraints honored

- H1 implementation still FORBIDDEN until Main disposes V001 (no V002, no source change).
- No CANNJudge submission by this route. Local % ≠ Official Score.
- Track-B research appended in `phase4/research/SCHED-ROWGROUP-X/next-hypotheses.md`.

## For Main

Candidate is measurement-clean under the registered protocol; ready for Main disposition
(online promotion decision or judge ownership assignment). 17×256 remains a neutral control
(in-noise), mechanism win is the unaligned 33×100 class.
