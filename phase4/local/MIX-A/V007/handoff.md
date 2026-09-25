# MIX-A V007 Route Handoff

## State

- Route: `MIX-A`
- Pending Candidate: `V007`
- Direct Parent: `MIX-A-V003`
- Parent Official Score: `44.69`
- Branch: `exec/sixlane-20260923-mix-a`
- Source commit before this handoff: `e2c4de7bdb1edbe4b907a883209aee3b6a9f5655`
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

- Support identities before this runner-only update: `support/CMakeLists.txt` SHA-256 `79eb81d32a9795d3395dab2d1854c0948d056394277ce5bd3e8680a89305a15e`; `support/runner_unified.asc` SHA-256 `9bd483c1162ff750b4784549b26fd28eb371d4b5e4a9775dfb509fb8a69d4a26`.
- Build inputs: V003 SHA-256 `1a1857a945ce1f04e3877437882b21a3d5071dddcd697103d7b539a0feb5c706`; V007 SHA-256 `a63ad29a997ae2fe8a1238c1a47a9d5ddfb14d16f725fca975ce5c55523f28eb`. The server3 source copies and unified runner matched these values for the successful builds.
- The existing server3 build tree is configured for CANN `8.5.0.alpha002`, Ascend910B3, `dav-2201`. Two earlier target builds reached generated registration compilation and failed at `<vector>`; their complete output remains in `support/build-unified.log`.
- The first successful build exported HCC paths through `CPATH` and `CPLUS_INCLUDE_PATH`. The same include setup is now encoded in `CMAKE_ASC_COMPILE_OBJECT` using `cmake -E env`, so the follow-up configure/build needed only `ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`. Compile and link both passed for `mix_a_v007_unified_probe` without an external include-path environment.
- The compile emitted existing `cce_global` ignored-attribute warnings for the `GM_ADDR` casts at runner lines 463-467; no compile or link error remained.
- Previous executable: `/home/data4t2/lelinfeng/phase4-workspaces/MIX-A/runner-v007/build/mix_a_v007_unified_probe`, ELF 64-bit AArch64 PIE, 650088 bytes, SHA-256 `bf82acf68ce38d28294e517f5b8fa2aace4cdf33252968e1f0c4b2676e4b0c29`. It was identified but not executed.
- The runner follow-up adds a V003-only `same-binary` mode: one ACL initialization/allocation set, at least 10 warmups, then two event-sampled blocks with at least 21 samples each. `.samples.tsv` labels both blocks; `.summary.tsv` contains each block and pooled full-set statistics, including MAD/median; `.qualification.tsv` records each block ratio, pooled ratio, block drift, parent correctness count, and protocol status. A parent correctness failure overrides the qualification status with `PARENT_CORRECTNESS_FAIL`.
- Qualification is `PASS` only when both block MAD/median values, pooled full-set MAD/median, and block drift are all at most 0.10. A value above 0.25 yields `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`; intermediate values yield `NEEDS_VALIDATION`.
- Both `same-binary` and `paired` require device, Main owner, lease ID, and a path to the current `server3-device-leases.tsv`. Before creating the ACL probe, the runner requires exactly one active `MIX-A` lease and exact device/owner/lease-ID matches, and rejects another active owner on the requested device. Device parsing rejects non-decimal, negative, non-representable, and out-of-range IDs; measurement modes also refuse d7 per protocol.
- CPU-only validation tests pass for valid device/lease records and mismatched device, Main owner, lease ID, released/duplicate leases, competing device ownership, malformed headers, and device bounds. The test compiles and runs without ACL headers or device access.
- `support/run_probes.sh` still invokes the older V003/V007 wall-clock binaries and samples `npu-smi`; do not use it for the unified procedure. No unified runner correctness invocation or NPU timing was made.
- `support/run_probes.sh` still invokes the older V003/V007 wall-clock binaries and samples `npu-smi`; do not use it for the unified procedure. No runner correctness invocation or NPU timing was made.

## Build Reproduction

- Existing build directory: `/home/data4t2/lelinfeng/phase4-workspaces/MIX-A/runner-v007/build`; it was reused without cleaning.
- The CMake compile rule supplies the four HCC include directories to both the driver and its child compile processes through `CPATH` and `CPLUS_INCLUDE_PATH`. This resolves `<vector>` in the generated registration unit.
- The successful target was built against the exact V003/V007 sources above. `support/build-unified.log` retains both earlier failures and subsequent successful configure, compile, and link output.
- Final runner SHA-256 `3eec1aa67a225761fda536d71c6f4f0bd2fcc01fbf2cb71cae0a26d3724173ee`; `runner_validation.h` SHA-256 `7ff5e0af8b2a0fb8247f9a41f93e28f548ef93f323df5e18765f927b984a4406`. The rebuilt AArch64 PIE is 669600 bytes with SHA-256 `c263fbde2347d554f50569a8154425b0694f4f092f0a84dd09265451843a2256`.
- `runner_validation_test.cpp` passed locally with `c++ -std=c++17 -Wall -Wextra -Werror`; it does not initialize ACL. The unified target compiled and linked on server3; its executable was not run.
- No ACL runner mode, NPU correctness run, or timing was run. Do not time without a current exclusive device lease.

## Lease History Follow-up (2026-09-25)

- `runner_validation.h` now folds the append-only lease table to the latest row for each unique `lease_id` before counting active leases or checking device ownership. A later `RELEASED` record therefore retires the earlier `LEASED` record.
- Host-only tests cover a released prior lease followed by a valid MIX-A lease on the same device, and rejection when another active lease conflicts on that device. Executed `c++ -std=c++17 -Wall -Wextra -Werror phase4/local/MIX-A/V007/support/runner_validation_test.cpp -o <temporary-binary> && <temporary-binary>`; exit code `0`, all assertions passed. The temporary binary was removed; no ACL headers or device runtime were used.
- The first rebuild attempt lacked `ASCEND_HOME_PATH` and failed during ASC compilation; that output is retained in `support/build-unified.log`. Rebuilding with `ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002` passed compilation and linking.
- V003 and V007 build-source SHA-256 values remain `1a1857a945ce1f04e3877437882b21a3d5071dddcd697103d7b539a0feb5c706` and `a63ad29a997ae2fe8a1238c1a47a9d5ddfb14d16f725fca975ce5c55523f28eb`. The unified runner SHA-256 remains `3eec1aa67a225761fda536d71c6f4f0bd2fcc01fbf2cb71cae0a26d3724173ee`; updated `runner_validation.h` SHA-256 is `f3b490d747a433725aac55a2dbce6c5d4d15f287d298af848a72e272162d2f37`.
- Rebuilt executable: `/home/data4t2/lelinfeng/phase4-workspaces/MIX-A/runner-v007/build/mix_a_v007_unified_probe`, AArch64 PIE, 674192 bytes, SHA-256 `dce996af2ec709219819de3e9ba908f0d41744f2b9820965a1385e080b949110`. It was identified but not executed.
- Track B review retains four distinct design candidates in `phase4/research/MIX-A/next-hypotheses.md`; the aligned `DataCopy` proposal was marked infeasible based on local CANN API guidance. No Candidate source was changed.

## Timing And Next Inputs

The four retained pairs remain `LOAD_CONTAMINATED`; preserve those labels. Their direction varies, so they do not establish a gain or regression. No V007 timing was run by this replacement worker.

The existing runner inputs are device 6, rows `1`, width `256`, dtype `0` (FP32), epsilon `1e-5`; both variants use the same `runner_main.inc` and deterministic host inputs. That runner uses wall-clock timing, 3 warmups, 11 samples, separate variant processes, and writes only a per-process median. It does not meet the current unified timing procedure.

After Main review and a fresh exclusive device lease, pass the current lease TSV and its exact device, Main owner, and lease ID to `same-binary` for V003 shape qualification, then use `paired` for interleaved V003/V007 pairs. The runner rejects missing, released, conflicting, or mismatched lease records before ACL initialization. Do not combine new samples with the retained `LOAD_CONTAMINATED` values.

## Stop Point

- SSH build access is confirmed through `cann-server3`; no active MAIN-1 exclusive device lease is present. Devices d0-d6 are busy, and d7 is excluded from performance measurement.
- No runner execution, NPU correctness run, or timing was performed. Do not time until Main confirms an exclusive lease.
- No `V008` created. No CANNJudge submission made.
- Stop here for Main review.
