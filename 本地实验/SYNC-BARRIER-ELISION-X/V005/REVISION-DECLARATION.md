# SYNC-BARRIER-ELISION-X V005

## Parent

- Direct Parent: `R31B V011` (current Local Best; V001-V004 did not promote).
- Parent SHA-256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Parent copy: `parent.asc`.

## Single change

Remove the FP16-path `AscendC::PipeBarrier<PIPE_V>()` after `ToFloat(valueLocal, xLocal, valid)` and before `Mul(xFp32, valueLocal, valueLocal, valid)` in `ProcessNarrowMidOverlap`. No other kernel operation or pipeline behavior is changed.

- Candidate: `submission.asc`.
- Candidate SHA-256: `92afad39c7bcfe3331150f5627ff996d81645f2d1230d6cb1a612660ee91e12e`.
- Exact source diff: compare `parent.asc` with `submission.asc`; the verified diff is one `PIPE_V` barrier deletion before `Mul(xFp32, valueLocal, valueLocal, valid)`.

## Gate status

- Compile: `PASS` on local host `hwnput3` at `2026-10-07T04:52:15Z`, using CANN `8.5.T8.0.B060` / compiler package `8.5.0.alpha002`, `dav-2201`. The generated registration compiler needed `CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward`; the direct CMake host include propagation edit is retained. See `logs/compile-local-20261007T043221Z.log`, `logs/compile-local-fix-20261007T044036Z.log`, and `logs/compile-local-env-fix-20261007T045158Z.log`.
- Correctness: `PASS`, device 3, five FP16 cases at widths 128, 256, 1024, 2048, and 4096; zero parent/candidate bit mismatches. See `logs/correctness-local-fixed-label-20261007T050230Z.log`.
- Local: numeric result `-15.233141%` from all 62 interleaved parent/candidate pairs, device 3, FP16 8x2048. Verdict `LOCAL_REJECTED`, load quality `LOW/NOISY`; run directions disagree and no Local Best promotion is made. See `local-result.json` and `logs/local-device3-fixed-label-20261007T050415Z.log`.
- Online: `NOT_RUN`.

## Evidence boundary

V005 result is complete and committed. `CURRENT_LOCAL_BEST` remains `R31B-V011`. All V005 artifacts remain under this route directory. No shared records, other Route worktrees, or Online state are in scope.
