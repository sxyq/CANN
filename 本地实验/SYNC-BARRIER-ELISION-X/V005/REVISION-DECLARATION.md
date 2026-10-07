# SYNC-BARRIER-ELISION-X V005

## Parent

- Direct Parent: `R31B V011` (current Local Best; V001-V004 did not promote).
- Parent SHA-256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Parent copy: `parent.asc`.

## Single change

Remove the FP16-path `AscendC::PipeBarrier<PIPE_V>()` after `ToFloat(valueLocal, xLocal, valid)` and before `Mul(xFp32, valueLocal, valueLocal, valid)` in `ProcessNarrowMidOverlap`. No other kernel operation or pipeline behavior is changed.

- Candidate: `submission.asc`.
- Candidate SHA-256: not computed.
- Exact source diff: compare `parent.asc` with `submission.asc`; the verified diff is one `PIPE_V` barrier deletion before `Mul(xFp32, valueLocal, valueLocal, valid)`.

## Gate status

- Compile: not started after the CMake include propagation fix. A bounded DNS check of the configured `cann-server3` endpoint at `2026-10-07T04:09:44Z` returned no address (exit 2), so no SSH or remote command was started. See `logs/server3-dns-check-20261007T040944Z.log`; the earlier SSH failure remains in `logs/server3-access-retry-20261007T031307Z.log`.
- Correctness: not run; Compile has not passed.
- Local: not run; `LOCAL_SCORE=NONE`. No timing, throughput, or Official-comparability claim exists.
- Online: `NOT_RUN`.

## Evidence boundary

All V005 artifacts remain under this route directory. No shared records, other Route worktrees, or Online state are in scope.
