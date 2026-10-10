# N03 V001 Revision Declaration

ROUTE=N03 VECTOR-ILP-DUALCHAIN-X
REVISION=V001
PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
PERFORMANCE_REFERENCE=R31B V011 / Official 45.16
SOURCE_PATH=本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V001/submission.asc
SOURCE_PARENT_PATH=线上结果/R31B/V011/submission.asc
SOURCE_SHA256_BEFORE=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
SOURCE_SHA256_AFTER=e97152453e04037e2d638c4a3c4d59b03793660f0b37deb1aea36d76de9b8f97

## Single change

Function: `ProcessWideLowPrecision`, BF16 pass-2 output path.

For a two-row batch, schedule the two rows' independent Vector chains in stages:
both `Muls`, then both `Mul`, then both `Add`. Preserve the existing arithmetic,
input/output memory traffic, reduction, event synchronization, conversion, and
row writeback order. The one-row and FP16 paths retain their existing order.

## Performance hypothesis

Each BF16 row currently issues a dependency chain of Vector operations before
starting the next row. Adjacent independent rows share the loaded gamma/bias
vectors; interleaving the chains may expose Vector instruction-level parallelism
and reduce dependency stalls without changing the amount of arithmetic or data
movement.

## Mechanism boundary

N03 studies Vector instruction-level parallelism only. No changes are made to
arithmetic, memory access, synchronization, tile sizing, ownership, or compute
amount. N01 register lifetime, N02 code layout, N04 GM/L2 streaming, and N05
Scalar address generation are outside this revision.

## Validation

COMPILE=KERNEL_TARGETS_PASS; device=submission=full_link; COMPILE_RC=0
COMPILE_SOURCE_FILE=single-version/R31B-V016-WIDE-TILE-SEED_kernel.asc
COMPILE_SOURCE_SHA256=e97152453e04037e2d638c4a3c4d59b03793660f0b37deb1aea36d76de9b8f97
RUNNER_LINK=FAIL; probe_v016; RUNNER_LINK_RC=2
RUNNER_LINK_ERROR=undefined symbols rtLinkedDevBinaryRegisterDelay and __cce_rtKernelLaunch
CORRECTNESS=NOT_RUN; existing runtime/paired runner calls aclrtResetDevice and NPU5 has active VLLMWorker_TP PID 3355594
LOCAL_SCORE=NONE; runtime validation was not run because the assigned NPU5 is occupied and runner reset is unsafe
DEVICE_ID=5
FREE_HBM_MB=5799; HBM_PRE=59737/65536; HBM_POST=59737/65536
AICORE_PRE=0%; AICORE_POST=0%; ACTIVE_PROCESS=3355594:VLLMWorker_TP:56358MB
GIT_COMMIT=TO_BE_RECORDED
NEXT_ACTION=No runtime validation in V001; use the existing paired runner only in a reset-safe device window.
