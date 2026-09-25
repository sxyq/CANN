# ALIGN-TAIL-X — 2×100 FP32 Qualification Runbook (MAIN-2, unified timing protocol)

Status: **PREPARED 2026-09-25 — no timing run yet**. Binaries built on cann-server3; device window is Main's to open. This runbook is preparation only; no lease line was appended this turn.

Protocol basis: `phase4/control/local-timing-protocol.md` (same-binary noise floor → interleaved P/C; device events primary; warmup ≥10; ≥21 samples/block; batch N=1) and `phase4/control/execution-contract.md` §R (Track-A) / §F (local-first flow).

## 1. Identity (verified 2026-09-25, local worktree + cann-server3)

| item | local path (under `cann-next6/ALIGN-TAIL-X/`) | SHA256 |
|---|---|---|
| Candidate V001 | `phase4/local/ALIGN-TAIL-X/V001/submission.asc` = `phase4/workspaces/ALIGN-TAIL-X/V001/kernel.asc` | `f573d16d39fb54a2a75f75f90943168b96dd97d83fb6bd094f11fd73c61df840` |
| Direct Parent PURE-R009-V001-ALIGNED-DATACOPY | `phase4/workspaces/ALIGN-TAIL-X/parent/PURE-R009-V001-ALIGNED-DATACOPY_kernel.txt` = `parent/kernel.asc` | `c8d0f010f8fc68b90d69c6d2bcbf76ec7bb927bd9dc4f0fcac32425e5288c63c` |

Server copies: `~/phase4-workspaces/ALIGN-TAIL-X/V001/kernel.asc` and `~/phase4-workspaces/ALIGN-TAIL-X/parent/kernel.asc` — same two SHA256 values.

Re-verify before any run:

```bash
cd /Users/sunyiyang/Desktop/Project/cann-next6/ALIGN-TAIL-X
shasum -a 256 phase4/local/ALIGN-TAIL-X/V001/submission.asc \
  phase4/workspaces/ALIGN-TAIL-X/parent/PURE-R009-V001-ALIGNED-DATACOPY_kernel.txt
ssh cann-server3 'cd ~/phase4-workspaces/ALIGN-TAIL-X && sha256sum V001/kernel.asc parent/kernel.asc'
```

## 2. Binaries and harness

Built 2026-09-25 on cann-server3 from the worktree sources above. Harness is a verbatim port of the SCHED reference (`runner_ref.inc`, md5 `93e6441937e9cd8394b734e124d18469`): one aclInit/setDevice/stream/malloc/H2D per process, warmup once, measurement blocks in the same process, `DEVICE_EVENT_US` primary + `HOST_WALL_US` secondary, batch N=1, CLI `argc` 10/11.

| binary | server path | local copy | SHA256 |
|---|---|---|---|
| Parent | `~/phase4-workspaces/ALIGN-TAIL-X/support/build/atx_ref_parent_probe` | `cann-next6/ALIGN-TAIL-X/phase4/workspaces/ALIGN-TAIL-X/support/build/atx_ref_parent_probe` | `a8bd66a6b6da5bdf1acf350edd8e6f55c8044a8db48d9421a650637285d3187c` |
| Candidate V001 | `~/phase4-workspaces/ALIGN-TAIL-X/support/build/atx_ref_v001_probe` | `.../support/build/atx_ref_v001_probe` | `bfb2988a482337a21e1ef2d2f5da3c81aaf7695956d1604f7e8c0b2e59a04955` |

Harness sources (worktree + server, same content): `support/{runner_ref.inc, runner_ref_parent.asc, runner_ref_v001.asc, local_types.h, CMakeLists.txt, build_ref_probes.sh}`. Rebuild: `ssh cann-server3 'cd ~/phase4-workspaces/ALIGN-TAIL-X/support && ./build_ref_probes.sh'` (cmake -j1, `CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/x86_64-linux-gnu/c++/11:/usr/include/c++/11/backward` plus the aarch64 dirs — cann-server3 is aarch64, the x86_64 dir does not exist there).

Start-proof (untimed, d6, 2026-09-25T10:03Z): `atx_ref_parent_probe 6 2 100 0 … 1 1 1 0` exited 0, `bad=0`. Smoke outputs under `support/build/smoke/` are not measurement data.

## 3. Environment (every run, non-login ssh)

```bash
ssh cann-server3
export LD_LIBRARY_PATH="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:/usr/lib/aarch64-linux-gnu"
```

Without this the loader fails on `libregister.so` (seen on SCHED's first ref invocation).

## 4. (a) Same-binary parent-vs-parent qualification

Preconditions: Main has appended the lease line (§7), `npu-smi info` shows the assigned device AICore 0%, and `ps aux | grep -E 'srx_|atx_|msprof'` shows no other timing process. Device = the leased device (`DEV` below, expected d4); never run while another Route's timing is active.

```bash
DEV=4   # value from the lease line
OUT=~/phase4-workspaces/ALIGN-TAIL-X/support/results-qual-2x100/d${DEV}
mkdir -p "$OUT"
npu-smi info > "$OUT/npu-smi-start.txt" 2>&1
date -Is > "$OUT/start.timestamp.txt"
cd ~/phase4-workspaces/ALIGN-TAIL-X/support/build
export LD_LIBRARY_PATH="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:/usr/lib/aarch64-linux-gnu"

# attempt 1: warmup 10, 2 in-process blocks x 31 samples, gap 2 s, DEVICE_EVENT primary
./atx_ref_parent_probe ${DEV} 2 100 0 "$OUT/sb1-parent-w10-s31" 10 31 2 2 \
  > "$OUT/sb1-parent.stdout" 2> "$OUT/sb1-parent.stderr"
```

Read `$OUT/sb1-parent-w10-s31-stats.txt` (`B1_DEVICE`, `B2_DEVICE`, `ALL_DEVICE` rows):

- `MAD/med` = `ALL_DEVICE / MAD_us ÷ median_us`
- `drift` = `|B1_DEVICE median_us − B2_DEVICE median_us| ÷ ALL_DEVICE median_us`

**Decision (protocol §Same-binary validation): PASS iff `MAD/med ≤ 0.10` AND `drift ≤ 0.10`.**

- FAIL → one more attempt (tag `sb2-…`, same command; **max 2 attempts**).
- Both attempts FAIL: if `MAD/med > 0.25` or `drift > 0.25` → `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` (harness/host issue — never a Candidate issue); else `NEEDS_VALIDATION`. Report to Main; no (b).
- PASS → continue to (b) in the same lease window.

Finish (a) with `npu-smi info > "$OUT/npu-smi-end.txt" 2>&1` and `date -Is > "$OUT/end.timestamp.txt"`.

## 5. (b) Interleaved P/C — only after (a) PASS

Same `DEV`, `OUT`, `LD_LIBRARY_PATH`. Four pair processes in sequence **P C C P ×2** (SCHED's pattern: each block contains one PC and one CP pair; one process per side; 31 samples/process; warmup 10):

```bash
P=./atx_ref_parent_probe; C=./atx_ref_v001_probe

# Block 1
$P ${DEV} 2 100 0 "$OUT/p1-2x100-P" 10 31 1 0 > "$OUT/p1-2x100-P.stdout" 2> "$OUT/p1-2x100-P.stderr"
$C ${DEV} 2 100 0 "$OUT/p1-2x100-C" 10 31 1 0 > "$OUT/p1-2x100-C.stdout" 2> "$OUT/p1-2x100-C.stderr"
$C ${DEV} 2 100 0 "$OUT/p2-2x100-C" 10 31 1 0 > "$OUT/p2-2x100-C.stdout" 2> "$OUT/p2-2x100-C.stderr"
$P ${DEV} 2 100 0 "$OUT/p2-2x100-P" 10 31 1 0 > "$OUT/p2-2x100-P.stdout" 2> "$OUT/p2-2x100-P.stderr"

# Block 2 (independent repeat of the same pattern)
$P ${DEV} 2 100 0 "$OUT/p3-2x100-P" 10 31 1 0 > "$OUT/p3-2x100-P.stdout" 2> "$OUT/p3-2x100-P.stderr"
$C ${DEV} 2 100 0 "$OUT/p3-2x100-C" 10 31 1 0 > "$OUT/p3-2x100-C.stdout" 2> "$OUT/p3-2x100-C.stderr"
$C ${DEV} 2 100 0 "$OUT/p4-2x100-C" 10 31 1 0 > "$OUT/p4-2x100-C.stdout" 2> "$OUT/p4-2x100-C.stderr"
$P ${DEV} 2 100 0 "$OUT/p4-2x100-P" 10 31 1 0 > "$OUT/p4-2x100-P.stdout" 2> "$OUT/p4-2x100-P.stderr"
```

Snapshot `npu-smi info` before the first and after the last process. Raw samples are permanent (`*-raw.tsv`); no sample deletion.

**Decision rules:**

- Per-pair delta `d_i = (C_i median_us − P_i median_us) / P_i median_us` from the `*_DEVICE` median in each `*-stats.txt`.
- Noise floor = same-binary `MAD/med` from (a) (a fraction, e.g. 0.052).
- Block 1 = p1+p2, Block 2 = p3+p4. Block-level delta = median of its two pair deltas.
- A block **favors Candidate beyond floor** iff its block-level delta `≤ −floor`.
- **`ONLINE_CANDIDATE` iff both blocks favor Candidate beyond floor.** Otherwise `NEEDS_ONE_MORE_LOCAL` (report the 4-pair table, favor count, and block-level deltas; no adverse claim from one-sided or within-floor results).
- Any `bad≠0` in a Candidate stats file stops the comparison (correctness first).

Per contract §F, only Main may return `MEASUREMENT_BLOCKED` / `NEXT_HYPOTHESIS` / `PARK`. Local numbers are never an Official Score.

## 6. Expected output directory

- Server: `~/phase4-workspaces/ALIGN-TAIL-X/support/results-qual-2x100/d<DEV>/`
  containing `*-raw.tsv`, `*-stats.txt`, stdout/stderr, timestamps, npu-smi snapshots, and (if run) `summary` written by the Route Agent.
- Copy back after the window closes:
  `rsync -av cann-server3:'phase4-workspaces/ALIGN-TAIL-X/support/results-qual-2x100/' /Users/sunyiyang/Desktop/Project/cann-next6/ALIGN-TAIL-X/phase4/local/ALIGN-TAIL-X/V001/support/results-qual-2x100/`

## 7. Lease line to append when Main opens the window

Tab-separated, appended to `phase4/control/server3-device-leases.tsv` **by Main** (this runbook turn appended nothing):

```text
<DEV>	MAIN-2	ALIGN-TAIL-X	SV-ALIGN-2x100	LEASED	<ISO8601Z>	-	exact-shape 2x100 FP32 same-binary then interleaved P/C (P C C P x2); unified device-event protocol; no source edit
```

Release line when the window closes:

```text
<DEV>	MAIN-2	ALIGN-TAIL-X	SV-ALIGN-2x100	RELEASED	<startZ>	<endZ>	<outcome: PASS/NEEDS_VALIDATION/MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE + P/C run or not>
```
