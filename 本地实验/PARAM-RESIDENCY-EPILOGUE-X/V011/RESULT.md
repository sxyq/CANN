# PARAM-RESIDENCY-EPILOGUE-X V011

ROUTE=PARAM-RESIDENCY-EPILOGUE-X
REVISION=V011
PARENT=V009
PARENT_SOURCE_COMMIT=2e7cc0a8
CANDIDATE_SOURCE=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V011/R31B-V016-WIDE-TILE-SEED-V011_kernel.asc
RUNNER=本地实验/R31B/V016/support/paired/paired_runner_v016
BUILD_TARGET=paired_runner_v016
PUSH=NO

## HYPOTHESIS

In V009 `ProcessWideLowPrecision`, wide FP16 uses one output tile and performs `SyncVToMTE3`, `Store`, and `SyncMTE3ToV` for every output tile. This makes the Vector side wait for each MTE3 transfer before it can write the next result. The same kernel already contains an alternating two-output-tile event pattern in `ProcessWideFp16BatchedOutputPipelined`: each slot has a `V_MTE3` ready event and an `MTE3_V` release event, and the release is waited only before that slot is reused.

V011 applies that existing pattern to the wide FP16 epilogue. Two output tiles allow MTE3 to drain one store while Vector forms the next result in the other slot. The arithmetic order remains `ToFloat -> Muls -> FromFloat -> Mul(Gamma) -> Add(Bias) -> Store`; Gamma/Bias load count, parameter values, tile traversal, and BF16 behavior are unchanged. V007 tested a single low-precision output tile for the BF16 path; this candidate tests two slots on the FP16 path.

Source evidence: V009 lines 82–89 allocate one wide FP16 output tile; lines 3325–3399 contain the current output path. The reusable two-slot event ordering is at lines 2916–3071 in the V009 kernel. V007's recorded Local result had mixed small medians and all P10–P90 ranges crossing zero, so it does not establish a stable gain for the single-slot schedule.

## CHANGE

- Copied the V009 kernel snapshot and changed only the wide FP16 output staging in `ProcessWideLowPrecision`.
- Allocated two FP16 output tiles and alternated their use across output stores; each slot is released after its MTE3 transfer completes.
- Counted the two output tiles in the existing wide-row UB sizing call. No math, Gamma/Bias handling, BF16 path, Runner, or build script was changed.

## COMPILE

STATUS=NOT_RUN
REASON=Server-side CANN build awaits user confirmation; no local build was attempted.
BUILD_TARGET=paired_runner_v016

## CORRECTNESS AND LOCAL

CORRECTNESS=NOT_RUN
LOCAL=NOT_RUN
LOCAL_SCORE=NONE
DEVICE=NOT_RUN

## NEXT DEVICE ENTRY

After server/NPU access is confirmed, build the existing `paired_runner_v016` target with this V011 source as the candidate input, then use the existing Runner entry `paired_runner_v016 --paired 7`. No Runner change was made in this revision.

