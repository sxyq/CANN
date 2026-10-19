# N04 V017 Results

## Current Candidate

- Route: `N04-GM-CACHE-STREAMING-X`.
- Direct parent: N04 V016, source commit `6bf56393e`.
- Current Candidate source commit: `6121b808b`.
- Source SHA-256: `786b0100c88adb232219fd221c4ffeea68ee393dc692405a41cbaf3cb97e646f`.
- Single change from V016: wide low-precision `xGm_` hint changes from `CACHE_MODE_DISABLE` to `CACHE_MODE_PERSISTENT`. `biasGm_` stays persistent; gamma, residual, and output hints are unchanged.
- Arithmetic, Vector instructions, Scalar indexing, tile traversal, DMA descriptors, and parameter residency are unchanged.

## Earlier V017 Input

An earlier Candidate at commit `026fbc655` changed `biasGm_` from persistent to disabled. Its Compile output, input identities, d7 snapshots, and binary remain under files named `*-bias-disable*`. That input is superseded and is not the current V017 Candidate; no Correctness or Local result was collected for it.

## Compile

- Host: `hwnput3` via `cann-server3`; CANN `8.5.0.alpha002`; SoC `Ascend910B3`.
- Parent: R31B V011 Champion, source SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate and existing Runner input identities are in `remote-inputs-x-persistent.txt`.
- CMake configure and Compile passed using the existing `paired_runner_v016` entry and the V016 CANN 8.5 environment. Compile return code 0; raw outputs are `configure-x-persistent.log` and `paired-build-x-persistent.log`.
- Built executable SHA-256: `bcedbbf3aab5447b2d277552679f06de86269a2f98baf43bcf608cada5f6d7a4`. Local artifact: `artifacts/paired_runner_v016-x-persistent`.
- A fresh independent configure/build used the same Candidate and Champion paths in `build-paired-reverified-786b`. The initial compile attempt failed because the plugin could not find `<vector>`; `build-current-sha-rebuild.log` preserves it. The retry with the established C++ `CPATH` passed; see `configure-current-sha-rebuild.log` and `build-current-sha-rebuild-cpppath.log`.
- Rebuilt executable SHA-256: `bcedbbf3aab5447b2d277552679f06de86269a2f98baf43bcf608cada5f6d7a4`. Local copy: `artifacts/paired_runner_v016-x-persistent-reverified-786b`. Remote source and CMake cache identities are recorded in `build-identity-current-sha-rebuild.txt`.

## Correctness

- Correctness: PASS for all four Champion/V017 cases on d7. Raw output is `correctness-current-sha-d7.log`; it ends with `RUNNER_COMPLETE mode=CORRECTNESS correctness=PASS cases=4 device=7`.
- Immediately before launch, d7 recorded AICore 7%, AIVector 0%, HBM bandwidth 5%, and PID 439848 using 21,260 MiB. The earlier launch-window read showed HBM bandwidth 57%; the later read was 5%. The post-Correctness snapshot recorded AICore 0%, AIVector 0%, and HBM bandwidth 61%.

## Paired Local

- Status: measured on d7 with the existing `paired_runner_v016`, 45 warmups and 21 alternating Parent/Candidate pairs per case. The complete raw log has 84 `SAMPLE` records, four `PAIR_BLOCK` summaries and a `RUNNER_COMPLETE` marker; it starts at `2026-10-11T23:39:53+00:00` and records remote return code 0. Parent is R31B V011 SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; Candidate is V017 SHA-256 `786b0100c88adb232219fd221c4ffeea68ee393dc692405a41cbaf3cb97e646f`. The local source, remote source, CMake input paths, rebuilt executable and saved Runner artifact identities are recorded in `remote-inputs-x-persistent.txt` and `build-identity-current-sha-rebuild.txt`.
- FP16 tail: Parent median 365.720 us, Candidate median 349.300 us; paired-delta median -16.080 us, 19/21 negative. Four large signed outliers widen the delta range to -383.240..337.380 us.
- FP16 wide: Parent median 925.280 us, Candidate median 879.900 us; paired-delta median -50.200 us, 19/21 negative. The delta mean is +1.910 us because of two positive outliers, including +867.700 us; the case is noisy.
- BF16 tail: Parent median 426.760 us, Candidate median 414.840 us; paired-delta median -12.960 us, 19/21 negative. The observed delta range is -31.400..11.360 us.
- BF16 wide: Parent median 2941.260 us, Candidate median 2871.820 us; paired-delta median -65.280 us, but only 12/21 pairs are negative. Delta mean is +250.482 us and range is -1509.980..1514.300 us, with strong bimodal timing variation.
- Overall `LOCAL_SCORE=NONE`: wide-case paired distributions, especially BF16 wide, have large variation, and the snapshots cannot establish the exact load during each sample. Preserve the negative tail signals and FP16-wide median as case-level observations only; this single Local run does not establish a stable overall gain.
- `LOAD_QUALITY=UNCERTAIN`: the launch snapshot was quiet, while nearby pre-run and post-run snapshots show active shared traffic; per-sample device-load telemetry was not captured. No specific outlier is attributed to the shared PID.
- Load context: the launch snapshot at `2026-10-11T23:39:28+00:00` showed AICore/AIVector/HBM bandwidth at 0%, with shared PID 439848 still using 21,260 MiB. Earlier reads at 23:34:38 and 23:35:24 showed HBM bandwidth 31% and 64%; the post-Local read at 23:41:26 showed AICore 12%, AIVector 4%, HBM bandwidth 59%, with the same PID. The log contains no per-sample utilization trace, so these readings indicate a shared-load risk but cannot attribute individual outliers to a specific process. The shared process was left untouched.

## Device and Disk State

- Compile-time d7 and server-space readings are in `pre-build-x-persistent-*` and `server-space-x-persistent-pre-build.txt`.
- The pre-rebuild snapshot recorded HBM use 37%, AICore/AIVector/HBM bandwidth at 0%, and PID 439848 using 21,260 MiB. The process was left untouched. Correctness ran after low-load reads. Local started only after a fresh launch snapshot again showed AICore/AIVector/HBM bandwidth at 0%; retain the earlier and post-run active readings as shared-load context.
- `/home/data4t2` reported 100% use with 16,572,432,384 bytes available after Compile. No unrelated files were removed or changed.

## Evidence Files

- Current input identity: `remote-inputs-x-persistent.txt`; parent identity: `remote-parent-identity-x-persistent.txt`.
- Configure/build and executables: `configure-x-persistent.log`, `paired-build-x-persistent.log`, `build-identity-x-persistent.txt`, `configure-current-sha-rebuild.log`, `build-current-sha-rebuild.log`, `build-current-sha-rebuild-cpppath.log`, `build-identity-current-sha-rebuild.txt`, `artifacts/paired_runner_v016-x-persistent`, and `artifacts/paired_runner_v016-x-persistent-reverified-786b`.
- Correctness, Local and device state: `correctness-current-sha-d7.log`, `local-paired-current-sha-d7.log`, `pre-rebuild-verify-d7-20261011T2320Z.txt`, all `pre-correctness-current-sha-*` files, `post-correctness-current-sha-d7.txt`, all `pre-local-current-sha-d7-*` files, and `post-local-current-sha-d7.txt`.
- Superseded bias-disable build evidence is retained under `*-bias-disable*` names and is not attributed to the current Candidate.
- Existing Runner inputs: `paired_runner_v016.asc`, `paired_runner.cpp`, `paired_runner_abi.h`, and `paired-CMakeLists.txt`.
