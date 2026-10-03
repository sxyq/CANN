# SYNC-TOPOLOGY-CHAMPION-X V001 Warmup-60 Retry

## Scope and trigger

- This retry keeps Candidate `submission.asc`, its SHA256 `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`, and Revision V001 unchanged.
- The prior Parent same-binary run used 45 synchronized warmups. Its two block medians were 10.98 us and 8.42 us; the first-to-second ratio is 1.304, above the protocol's 1.2 warmup-extension trigger.
- Use 60 synchronized warmups for Parent same-binary, each Parent window process, and each binary in paired timing. The shared `kWarmups` constant sets all three phases, and each jitter header reports that value.
- Keep the prior `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` result and its raw/jitter evidence unchanged. The retry starts a new qualification attempt; historical samples do not count as a pass.

## Fixed measurement scope

- Route/revision: `SYNC-TOPOLOGY-CHAMPION-X` / `V001`.
- Device: d4. Shape: `[2, 12288]`. Dtype: FP16.
- Direct Parent: `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate source SHA256: `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`.
- Reuse the exact-source timing target and its existing Parent/Candidate modules after verifying their recorded identities. Do not alter the Candidate source or this Revision identity.

## Build

After pushing the runner change, rebuild only the timing target in the existing server build directory. Store build output and the new executable identity in a fresh `results/<RUN_ID>/` directory. Do not invoke any runner mode during this build step.

```bash
REMOTE_ROOT=/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001
RESULT_DIR="$REMOTE_ROOT/results/$RUN_ID"
source /usr/local/Ascend/ascend-toolkit/set_env.sh
cmake --build "$REMOTE_ROOT/build-timing" \
  --target sync_topology_v001_timing --verbose -j2 \
  > "$RESULT_DIR/build-timing.log" 2>&1
sha256sum "$REMOTE_ROOT/build-timing/sync_topology_v001_timing" \
  > "$RESULT_DIR/timing-executable-identity.txt"
```

Build outcome and executable SHA256: pending.

## Measurement order and admission

Do not start timing until Main records a new d4 lease in the canonical lease log and grants the live preflight for this attempt. Immediately before timing, capture `npu-smi info`, disk availability, and process inventory. Do not stop or alter resident processes.

1. Parent same-binary: one process, 60 synchronized warmups, then two blocks of 31 device-event samples. Require full-sample MAD/median <= 0.10 and block drift <= 0.10. Stop if it fails.
2. Only after same-binary PASS, run Parent window qualification: PRECHECK-A and PRECHECK-B, three fresh processes per block, 60 synchronized warmups and 21 device-event samples per process. Require both blocks to meet CV <= 0.15 and max/min <= 1.30. Stop if either fails.
3. Only after both qualifications pass and the lease remains active, run four groups of 11 adjacent Parent/Candidate pairs, alternating pair order and using 60 synchronized warmups for each binary.

Retain every new raw sample, jitter summary, identity record, and pre/post load snapshot in this retry's fresh result directory. Never reuse or overwrite the prior result directory or historical raw/jitter files.
