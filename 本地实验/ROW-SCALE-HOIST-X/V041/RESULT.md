# V041 Result

- Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`)
- Change: In `ProcessWideFp32FullCacheRows`, move each row's existing FP32 scale `Muls` before gamma `Mul`.
- Compile: PASS; `device` and `submission` targets.
- Correctness gate: `INVALID_UNRESOLVED`. At FP32 `[128,16384]`, the Parent failed with matched ratio `0.237092972` and max error `4.10239983`; Candidate attempts failed with matched ratios `0.189493656` and `0.202261448`, with max errors `4.00334167` and `3.87787485`.
- Diagnostic: Parent and Candidate also both failed at nearby FP32 `[128,12288]`, while dispatching `ProcessWideFp32FullCacheRows`. The harness supplied matching 2-D input/residual/output metadata, 1-D gamma/bias metadata, FP32 dtype 0, and 40 available cores. The shared failure is not attributed to Candidate; the wide-path/output issue remains unresolved.
- Local: `NOT_RUN_CORRECTNESS_GATE_FAILED`; no score or delta. `CURRENT_LOCAL_BEST=V026`.
- Official score absent; Online not run.

Evidence: compile and primary correctness logs, `correctness-candidate-capture-note.md`, the nearby-shape correctness log, `correctness-result.json`, `local-result.json`, source hash/metadata, and `diff.patch` are preserved in this directory.
