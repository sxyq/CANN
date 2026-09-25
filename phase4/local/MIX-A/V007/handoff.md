# MIX-A V007 Route Handoff

## State

- Route: `MIX-A`
- Pending Candidate: `V007`
- Direct Parent: `MIX-A-V003`
- Parent Official Score: `44.69`
- Branch: `exec/sixlane-20260923-mix-a`
- Source commit before this handoff: `108272cc1774de0d0b02efddddda7a4a0d133e89`
- Candidate source SHA-256: `a63ad29a997ae2fe8a1238c1a47a9d5ddfb14d16f725fca975ce5c55523f28eb`

## Source Lineage

The V003 source in the route workspace and its retained online record both have SHA-256 `1a1857a945ce1f04e3877437882b21a3d5071dddcd697103d7b539a0feb5c706`. The retained V003 result records 15/15 passing cases and Official Score `44.69`.

The V007 source in the route workspace and `phase4/local/MIX-A/V007/submission.asc` both have SHA-256 `a63ad29a997ae2fe8a1238c1a47a9d5ddfb14d16f725fca975ce5c55523f28eb`. The sidecar records the same value.

The direct source diff keeps V003 dispatch and math. The only executable-code delta removes the `SyncVToMTE2()` immediately before the x/residual loads in `ProcessNarrowMidFast`; header comments also identify the new revision and hypothesis. The single-row dispatch condition remains unchanged.

## Build And Executables

- Retained build records: device compile PASS, submission compile PASS, device link PASS, submission link PASS; CANN `8.5.0.alpha002`, Ascend910B3, `dav-2201`.
- Remote build directory: `/home/data4t2/lelinfeng/phase4-workspaces/MIX-A/runner-v007/build`.
- `mix_a_v003_probe`: AArch64 ELF, 512360 bytes, SHA-256 `728614abd8d0e6d10c8d7494e9e2704f49fd484d534e8586708f8e2ccd61a755`.
- `mix_a_v007_probe`: AArch64 ELF, 512336 bytes, SHA-256 `2443eb9b124e2a82c3750395a4feb3e273e3a3f808b50feda718074e742edb46`.
- CMake dependency records point the V003 binary to `MIX-A-V003-submission.asc` and the V007 binary to `MIX-A-V007-submission.asc`. Both remote source copies match their route SHA above.
- The remote `runner_main.inc`, `runner_v003.asc`, `runner_v007.asc`, `local_types.h`, `CMakeLists.txt`, and `run_probes.sh` have the same SHA-256 values as the retained route files.

## Timing And Next Inputs

The four retained pairs remain `LOAD_CONTAMINATED`; preserve those labels. Their direction varies, so they do not establish a gain or regression. No V007 timing was run by this replacement worker.

The existing runner inputs are device 6, rows `1`, width `256`, dtype `0` (FP32), epsilon `1e-5`; both variants use the same `runner_main.inc` and deterministic host inputs. That runner uses wall-clock timing, 3 warmups, 11 samples, separate variant processes, and writes only a per-process median. It does not meet the current unified timing procedure.

After Main review and a fresh exclusive device lease, retain the same shape and data inputs, use the leased device, and run the unified method: device events as the primary duration, at least 10 warmups and 21 samples, with interleaved V003/V007 pairs. Establish the per-shape same-binary noise floor first. Do not combine new samples with the retained `LOAD_CONTAMINATED` values.

## Stop Point

- Current lease table contains no `MIX-A` assignment; all visible entries are unrelated or released.
- Do not measure until Main confirms an exclusive lease and approves the paired run.
- No `V008` created. No CANNJudge submission made.
- Stop here for Main review.
