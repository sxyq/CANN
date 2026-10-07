# Exact V027 Parent C15 retry

- ROUTE / REVISION: `R-W4-4 / MODE-DISPATCH-CUTOFF-X / V028`
- DIRECT PARENT: V027; exact source hash check passed.
- HOST: `hwnput3` (current local host; no SSH used).
- DEVICE: 4, 910B3.
- SHAPE / DTYPE: C15, `1x32768`, FP32.
- START / END: `2026-10-07T06:03:40.751336573Z` / `2026-10-07T06:03:52.335175510Z`.
- COMMAND: `clx_ref_parent_probe 4 1 32768 0 <prefix> 0 1 1 0 1` (exact existing probe; one invocation).
- RESULT: return code `3`; `bad=28174`; `max_abs=1.2031`.

## Source and executable identity

- `parent.asc`: `f0ab43545e13d943e4c5bb5ff22400426e18f560ab313c1c6856b39fa904f033`.
- `runner_ref.inc`: `2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7`.
- `runner_ref_parent.asc`: `ce5e6b47de1ff74741b57b24e817aac10a4968e52a41c168c4f1d2627d619910`.
- `main.asc`: `d5dcc6fdf961bb455ee47ec27a265607f585c6b2bbc1f51188479f19cf417ee1`.
- `local_types.h`: `00cdbb292bd146bdbf47814afb1a7cb8937f8eb0cf32581c71c98ab1b32cc63e`.
- `CMakeLists.txt`: `7187649eb99f7de3aabaca38334252b37e7efc54d7e533eef8681015ba210fb3`.
- Existing `clx_ref_parent_probe`: `ddb9e0ba4f628380c9be9028f9d7c9cdc5dc89aadf6a66e14d50712054a26319` (ELF AArch64, 480280 bytes).
- The route-bound Candidate was not invoked or modified. No compile or executable rebuild was performed.

## Device and process context

- Device 4 launch-pre: HBM capacity `65536 MB`, reported HBM usage `90%`, AICore `0%`, AIVector `0%`; process `VLLMEngineCor`, PID `2999855`, `55666 MB`.
- `npu-smi info -t memory` exposes capacity but not exact free MB on this host. Admission used a conservative one-percentage-point upper allowance (`91%` usage), giving a free-HBM lower bound of `5898 MB`, above the required `100 MB`.
- Device 4 post: HBM `90%`, AICore `0%`, AIVector `0%`; the same PID and process memory remained. Device 7 post snapshot is also retained; its Python PID `439848` remained present and was not altered.
- All-device pre HBM/AICore/AIVector and per-device NPU process snapshots are retained as `device[0-7]-usages-pre.txt` and `device[0-7]-proc-mem-pre.txt`; complete host process snapshots are retained before and after.
- Disk availability was `393 GB` before and after. Toolkit environment resolved to `/usr/local/Ascend/ascend-toolkit/latest`.

## Disposition

The exact V027 Parent failed again on C15 under the existing probe, on a different device with lower recorded AIVector use than the previous device-7 run. This is a genuine correctness-baseline blocker for V028, not a performance-measurement blocker. Stop this revision's experiment loop: no Candidate correctness, no Local, no repaired Parent, no V029, no Online action, and no shared-record write. Preserve this failed confirmation with the preceding exact-source Parent failures.

Raw samples, statistics, stdout/stderr, command, admission evidence, source/executable identities, runtime environment, device snapshots, process snapshots, timestamps, and disk snapshots are retained in this directory.
