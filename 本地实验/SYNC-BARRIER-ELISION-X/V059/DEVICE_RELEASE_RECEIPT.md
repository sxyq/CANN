# DEVICE_RELEASE_RECEIPT — SYNC-BARRIER-ELISION-X V059

- Device: NPU 3, exclusively assigned to this Route for V059 correctness and Local through raw capture.
- Capture complete: seven-case Correctness PASS and 62 interleaved Local pairs; all raw logs and load snapshots retained.
- Post-capture snapshot: `logs/device3-postcapture-release.log`, at `2026-10-08T07:55:00Z`; HBM `3,428/65,536 MB`, AICore `0%`, no NPU-3 process.
- Explicit release: `2026-10-08T07:55:01Z`, recorded in `logs/device3-postcapture-release.log`.
- No existing process was modified. No shared lease/device TSV was written.
- No further V059 device operation will be run.
