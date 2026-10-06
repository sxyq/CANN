# V001 local blocker

- ROUTE: RSQRT-VECTOR-INSTR-X
- REVISION: V001
- BUILD: PASS; exact Candidate SHA `d5af5c317aa4ec6d87dafae587e3b75aece44126e75672ffaa9ba7794dff770d`; executable SHA `1760e320a0e031fd90f64bcb680094f6900833942564abc425a4ea155755f770d`.
- CORRECTNESS: PASS on device 2 using the existing case0 (FP16 1x64), 64/64 matched, max abs `0.0009765625` under the existing `rtol=1e-3, atol=1e-3` gate. The project runner's numpy generation step was unavailable; no system package was installed. The exact existing case0 input/golden was run directly and checked with the Python standard library.
- LOCAL: `MEASUREMENT_BLOCKED`.
- BLOCK_REASON: The V001 evidence directory contains only the existing correctness case0 and no canonical benchmark runner or official case set. Therefore no valid Parent/Candidate same-binary qualification or interleaved timing pairs can be produced without inventing benchmark inputs or a scorer basis.
- LOCAL_SCORE: `N/A`; no score was fabricated.
- DEVICE: 2 preferred.
- ONLINE: forbidden by task.

## 2026-10-06 retry

- `V001` remains on the requested provenance: commit `e836bb83283327fd8df85ef3ace23433ab49735a`, with no Candidate source edit.
- Build and correctness remain PASS from the committed evidence.
- Route-local tooling audit: no committed performance runner, `runner_main.inc`, device-event host wrapper, canonical benchmark case set, raw samples, or jitter files exists under this V001 directory. The only executable test harness is the existing correctness case0 (`FP16_1x64`), which is not a valid performance runner.
- Live resource snapshot at `2026-10-06T17:02:37Z` on `hwnput3`, device 2: HBM `60210/65536 MB`, approximately `5326 MB` free; AICore `0%`; resident process `VLLMWorker_TP` PID `92813` using `56703 MB`. HBM admission is satisfied, and the resident process is record-only under the current protocol.
- No Parent/Candidate same-binary device-event measurement was run. Therefore no valid raw latency, throughput, delta, jitter, quality, or Local Score exists.
- `LOCAL_VERDICT=MEASUREMENT_BLOCKED`; `RETRY_REQUIRED=YES_REQUIRES_COMMITTED_ROUTE_LOCAL_RUNNER_AND_CANONICAL_CASE_SET`.
