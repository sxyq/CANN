# R31B V016 paired local probe preparation

State: host-side preparation only. Do not invoke the harness until Main explicitly grants R31B the uncontested device lease and server3 device 4 has a clear comparable load window. The executable launcher is `paired-probe-harness.sh`; it refuses to contact server3 unless `MAIN_DEVICE_LEASE=R31B` and `R31B_LOAD_WINDOW=CLEAR` are set.

## Fixed identities and runner

- Route/revision: R31B/V016; direct parent: R31B-V011.
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate source SHA256: `9f5c353e65a13a740fe97dc7e6415df032d27560831a3ad142c77592b8208eb5`.
- Host/device: `cann-server3`/device 4; rows 2; warmup 1; timed repeats 3.
- Existing server-side runner build: `/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/probe-build`, with `probe_v011` and `probe_v016`. Both use the same `runtime_probe.cpp` and CMake configuration; the parent binary selects the V011 source via `R31B_PROBE_PARENT=1`.
- Actual executable SHA captured on server3 at `2026-09-24T13:08:35+0800`: `probe_v011=78917340dab5c0723c2d18faf493dca32f0e3412f78971c3ef7daa4db1427891`; `probe_v016=81a2ebfac7d3ec0cf1cf65105c7b81cc995d16ca8d57ed072eca3bdba40218b2`.
- Runtime environment: source `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh`; set `/usr/lib/aarch64-linux-gnu` before toolkit runtime libraries in `LD_LIBRARY_PATH` as in the accepted route-local runner setup.

## Cases and interleaving

Keep the four established dtype/width cases unchanged. Within each case, use exactly two parent/candidate pairs in `P/C/P/C` order. Each executable invocation has the same shape/device/warmup/repeats arguments and internally reports the median of three timed repeats.

| Case | dtype arg | dtype | rows | width | device | warmup | repeats |
|---|---:|---|---:|---:|---:|---:|---:|
| `fp16-tail-d12288` | 1 | FP16 | 2 | 12288 | 4 | 1 | 3 |
| `fp16-wide-d32768` | 1 | FP16 | 2 | 32768 | 4 | 1 | 3 |
| `bf16-tail-d12288` | 2 | BF16 | 2 | 12288 | 4 | 1 | 3 |
| `bf16-wide-d32768` | 2 | BF16 | 2 | 32768 | 4 | 1 | 3 |

For each case, set `DTYPE` and `WIDTH` from the table. The exact four-invocation order is:

```sh
"$V011" "$DTYPE" 2 "$WIDTH" 4 1 3  # P1
"$V016" "$DTYPE" 2 "$WIDTH" 4 1 3  # C1
"$V011" "$DTYPE" 2 "$WIDTH" 4 1 3  # P2
"$V016" "$DTYPE" 2 "$WIDTH" 4 1 3  # C2
```

Record each raw output line separately; do not average cases together. The launcher records the source and executable SHA before the first case and refuses mismatched artifacts.

## Device-window decision


The launcher takes read-only device and process snapshots before and after every P/C/P/C case:

```sh
capture_load() {
  TZ=Asia/Shanghai date '+%Y-%m-%dT%H:%M:%S%z'
  npu-smi info -t usages -i 4
  ps -eo pid=,comm=,args= | grep -E '[Vv][Ll][Ll][Mm]|EngineCore|Worker_TP' | grep -v grep | sort -n
}
```

Record VLLM service/worker residency and correlate it with AICore, AIVector, HBM and HBM-bandwidth readings; process residency by itself does not prove active device work. Do not time if Main has not assigned the unique-device slot to R31B, if device usage is sustained or changing materially, if HBM remains heavily occupied, or if VLLM-driven device activity is visible. If the window is unclear, keep the run unstarted or label collected samples `LOAD_CONTAMINATED` and stop.

## Host-side command sequence (not yet run)

After explicit Main scheduling and a clear load window, invoke the local launcher from the R31B worktree. This command is intentionally not runnable under the current no-lease state:

```sh
MAIN_DEVICE_LEASE=R31B R31B_LOAD_WINDOW=CLEAR \
  ./phase4/local/R31B/V016/paired-probe-harness.sh
```

The launcher uses `DTYPE=1` for FP16 or `DTYPE=2` for BF16; `WIDTH=12288` for the tail case and `WIDTH=32768` for the wide case. It performs no remote call until both explicit markers pass.

## Results template

The launcher writes append-only evidence at `phase4/local/R31B/V016/paired-probe-rerun-<local-start>.txt`; it refuses to overwrite an existing path. Each record contains route/revision/parent, source and executable SHAs, lease markers, device/shape/runtime parameters, snapshots, exact P/C/P/C commands, and raw outputs.

For each case, let `P1`/`P2` be the parent medians and `C1`/`C2` be the candidate medians. The launcher parses `median_us=` from each runner line and records:

- `PARENT_JITTER_US=abs(P2-P1)` and `CANDIDATE_JITTER_US=abs(C2-C1)`; percentage jitter uses the corresponding two-run mean.
- `PAIR_1_DELTA_US=C1-P1` and `PAIR_2_DELTA_US=C2-P2`; positive means the candidate is slower.
- `MEDIAN_DELTA_US=(PAIR_1_DELTA_US+PAIR_2_DELTA_US)/2` (the median for two paired deltas).
- `WORST_DELTA_US=max(PAIR_1_DELTA_US,PAIR_2_DELTA_US)` and `WORST_ABS_DELTA_US=max(abs(PAIR_1_DELTA_US),abs(PAIR_2_DELTA_US))`.

| Case | P1/C1 medians (us) | P2/C2 medians (us) | parent jitter (us) | candidate jitter (us) | median delta (us) | worst delta (us) | load label |
|---|---|---|---:|---:|---:|---:|---|
| FP16 D=12288 | pending | pending | pending | pending | pending | pending | pending |
| FP16 D=32768 | pending | pending | pending | pending | pending | pending | pending |
| BF16 D=12288 | pending | pending | pending | pending | pending | pending | pending |
| BF16 D=32768 | pending | pending | pending | pending | pending | pending | pending |

Decision remains `NEEDS_ONE_MORE_LOCAL` until Main reviews valid, comparable local evidence. No source edits, new revision, or Online submission are in scope.
