# W4-R03 V001 Result

```text
ROUTE = W4-R03
REVISION = V001
DIRECT_PARENT = R31B-V011
CHANGE = Omit xFp32Buf_ and residualFp32Buf_ InitBuffer calls only for FP32, aligned small-row batches where every block handles at least two rows.
PARENT_SHA256 = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256 = 9259c38d104fa23046b7b3c95eeeef639e07dca89c2fd95499742afe3566a273
COMPILE = PASS
COMPILE_COMMAND = cmake --build build --target r03_v001_runner --parallel 1 -- -B
TOOLCHAIN = CANN 8.5.0.alpha002; bisheng 15.0.5; Ascend910B3 / dav-2201
CORRECTNESS = PASS; parent and candidate match the FP32 CPU reference and each other element-for-element.
CORRECTNESS_COMMAND = build/r03_v001_runner --correctness
LOCAL_SHAPE = 80x2056 FP32
DEVICE = 0; VECTOR_CORES = 40; BLOCKS = 40
FREE_HBM_BEFORE_MB = 27525 (derived from 65536 MB capacity and 58% use)
FREE_HBM_AFTER_MB = 27525 (same reported capacity/use)
LOCAL_PAIRS = 21; WARMUP_PAIRS = 45; ORDER = alternating P,C / C,P
LOCAL_COMMAND = build/r03_v001_runner --paired
PARENT_MEDIAN_EVENT_US = 36.820
CANDIDATE_MEDIAN_EVENT_US = 32.480
PAIRED_DELTA_MEDIAN_US = -2.780
LOCAL_DELTA_PERCENT = -11.787064
PARENT_EVENT_CV_PERCENT = 95.086365
CANDIDATE_EVENT_CV_PERCENT = 66.721027
ORDER_MEDIAN_DELTA_US = P,C:-2.780; C,P:-3.150
LOCAL_VERDICT = OBSERVATION_ONLY; one paired run has a broad spread and a long tail, so it does not establish a repeatable gain.
CURRENT_LOCAL_BEST = NONE
OFFICIAL = NOT_SUBMITTED
```

The only Candidate/source difference is the guarded omission of two FP32 scratch-buffer reservations. The dispatch predicates ensure every block follows `ProcessSmallFp32ContiguousBatched` or `ProcessSmallFp32Batched`; those paths do not acquire either buffer. Arithmetic, row ownership, other dtypes, and other dispatch paths remain unchanged.

Correctness used the repository FLOAT32 mixed tolerance (`atol=1.52587891e-5`, `rtol=9.765625e-4`, required matched ratio 0.99, max absolute error limit 0.01). Both outputs had matched ratio 1.0 and maximum absolute error `4.76837158e-7`; parent/candidate output difference was zero across 164480 elements.

Local context on device 0 was variable: pre-run AICore/AIVector use was 4%/5%, post-run 61%/54%; HBM bandwidth was 21%/11%. All raw event and wall samples are retained in `support/local.log`. Parent event samples ranged from 10.740 to 206.120 us; Candidate samples ranged from 14.080 to 102.000 us. The per-order paired-delta medians are close, but dispersion remains large. No same-binary P/P series was run.

Evidence files:

- `submission.asc` — Candidate source.
- `parent.asc` — exact R31B-V011 source used for comparison.
- `support/compile.log` — initial environment/build failures and final successful build.
- `support/correctness.log` — Parent/Candidate correctness output.
- `support/local.log` — full paired raw samples and summaries.
- `support/CMakeLists.txt`, `support/runner.asc`, `support/runner.cpp`, `support/runner_abi.h` — this Route's direct-call build and test harness.

The Local numbers are a single noisy observation, not a validated Local Best. V001 is one distinct R03 performance Candidate; a second distinct Candidate remains to be developed in this Route.
