# V052 Device Assignment

- Device: NPU 0, exclusive to ROW-SCALE-HOIST-X V052 for Parent/Candidate Correctness and, only if both pass, Local through raw result capture.
- Assignment source: Planning directive received 2026-10-08.
- Preconditions: V052 Compile PASS; fresh live FREE_HBM/process snapshot immediately before NPU use; proceed if FREE_HBM >= 100 MB.
- Safety: leave all existing processes untouched; retain raw Parent/Candidate outputs and all Local samples; explicitly release device 0 after numeric result capture.
- Status: RELEASED after numeric result capture at `2026-10-08T07:46:01Z`; see `device-release.log`.
- Post-capture: device 0 HBM `4814/65536 MB` used, AICore `0%`; no V052 runner remained. Existing Python PID `251901` (1434 MB) was left undisturbed.
