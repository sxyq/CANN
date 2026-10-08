# V057 Device Selection

- Selection snapshot: `device-snapshot-selection-20261008T160846Z.log`, captured on `hwnput3` at `2026-10-08T16:08:46Z`.
- Device 2 is excluded because MODE-DISPATCH V093 currently holds it, even though the snapshot shows no active NPU process there.
- Device 3 is selected: HBM capacity `65,536 MB`, usage `5%` (approximately `62,259 MB` free, comfortably above `100 MB`), AICore `0%`, AIVector `0%`, and `npu-smi info -t proc-mem -i 3` reports `No process in device`.
- No other active NPU process/Studio was listed on device 3. All observed processes and activity on other devices were left untouched.
- Capture a fresh device-3 process/HBM snapshot immediately before Parent/Candidate Correctness and again before Local. Recheck no process is listed; if the device is occupied, do not run on it.
