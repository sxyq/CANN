# MIX-A V007 Route Handoff

## State

- Route: `MIX-A`
- Pending Candidate: `V007`
- Direct Parent: `MIX-A-V003`
- Parent Official Score: `44.69`
- Branch: `exec/sixlane-20260923-mix-a`
- Source commit before this handoff: `9206c5be6467974b7f49db26cdc67645294349fb`
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

## Unified Runner Build Review (2026-09-25)

- Local support identities: `support/CMakeLists.txt` SHA-256 `79eb81d32a9795d3395dab2d1854c0948d056394277ce5bd3e8680a89305a15e`; `support/runner_unified.asc` SHA-256 `9bd483c1162ff750b4784549b26fd28eb371d4b5e4a9775dfb509fb8a69d4a26`.
- Build inputs: V003 SHA-256 `1a1857a945ce1f04e3877437882b21a3d5071dddcd697103d7b539a0feb5c706`; V007 SHA-256 `a63ad29a997ae2fe8a1238c1a47a9d5ddfb14d16f725fca975ce5c55523f28eb`. The server3 source copies and unified runner matched these values for the successful builds.
- The existing server3 build tree is configured for CANN `8.5.0.alpha002`, Ascend910B3, `dav-2201`. Two earlier target builds reached generated registration compilation and failed at `<vector>`; their complete output remains in `support/build-unified.log`.
- The first successful build exported HCC paths through `CPATH` and `CPLUS_INCLUDE_PATH`. The same include setup is now encoded in `CMAKE_ASC_COMPILE_OBJECT` using `cmake -E env`, so the follow-up configure/build needed only `ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`. Compile and link both passed for `mix_a_v007_unified_probe` without an external include-path environment.
- The compile emitted existing `cce_global` ignored-attribute warnings for the `GM_ADDR` casts at runner lines 426-430; no compile or link error remained.
- Final executable: `/home/data4t2/lelinfeng/phase4-workspaces/MIX-A/runner-v007/build/mix_a_v007_unified_probe`, ELF 64-bit AArch64 PIE, 650088 bytes, SHA-256 `bf82acf68ce38d28294e517f5b8fa2aace4cdf33252968e1f0c4b2676e4b0c29`. It was identified with `file`, `readelf`, and `sha256sum`; it was not executed.
- Static runner review: paired mode uses one process, 10 warmups per variant, single-launch device events, alternating V003/V007 order, and performs D2H/reference comparison after the measured loop. Each pair contains one event sample per variant. Raw `.samples.tsv` records pair number and order; `.summary.tsv` reports per-variant aggregates, so per-pair deltas must be derived from raw rows and reported separately.
- Runner input risk, not changed under the build-only scope: `ParseShape` rejects negative device IDs, while both modes later narrow the parsed 64-bit value to `int` (`runner_unified.asc` lines 555-557, 596-597, 642-643). Reject values above `INT_MAX` before any future runner execution; an out-of-range conversion is implementation-defined and could select an unintended device.
- Same-binary readiness gap: `noise-floor` emits one in-process sample set per invocation and cannot report two block medians or their drift inside one process. Multiple invocations reinitialize ACL and are not a substitute for the timing protocol's in-process blocks. Extend this mode before using it for shape qualification.
- `support/run_probes.sh` still invokes the older V003/V007 wall-clock binaries and samples `npu-smi`; do not use it for the unified procedure. No runner correctness invocation or NPU timing was made.

## Build Reproduction

- Existing build directory: `/home/data4t2/lelinfeng/phase4-workspaces/MIX-A/runner-v007/build`; it was reused without cleaning.
- The CMake compile rule supplies the four HCC include directories to both the driver and its child compile processes through `CPATH` and `CPLUS_INCLUDE_PATH`. This resolves `<vector>` in the generated registration unit.
- The successful target was built against the exact V003/V007 sources above. `support/build-unified.log` retains both earlier failures and subsequent successful configure, compile, and link output.
- No correctness or timing mode was run. Do not time without a current exclusive device lease.

## Timing And Next Inputs

The four retained pairs remain `LOAD_CONTAMINATED`; preserve those labels. Their direction varies, so they do not establish a gain or regression. No V007 timing was run by this replacement worker.

The existing runner inputs are device 6, rows `1`, width `256`, dtype `0` (FP32), epsilon `1e-5`; both variants use the same `runner_main.inc` and deterministic host inputs. That runner uses wall-clock timing, 3 warmups, 11 samples, separate variant processes, and writes only a per-process median. It does not meet the current unified timing procedure.

After Main review and a fresh exclusive device lease, retain the same shape and data inputs, use the leased device, and run the unified method: device events as the primary duration, at least 10 warmups and 21 samples, with interleaved V003/V007 pairs. Establish the per-shape same-binary noise floor first. Do not combine new samples with the retained `LOAD_CONTAMINATED` values.

## Stop Point

- SSH build access is confirmed through `cann-server3`; no active MAIN-1 exclusive device lease is present. Devices d0-d6 are busy, and d7 is excluded from performance measurement.
- No runner execution, NPU correctness run, or timing was performed. Do not time until Main confirms an exclusive lease.
- No `V008` created. No CANNJudge submission made.
- Stop here for Main review.
