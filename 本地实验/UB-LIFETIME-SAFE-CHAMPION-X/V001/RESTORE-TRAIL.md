# V001 Restore Trail

ROUTE: `UB-LIFETIME-SAFE-CHAMPION-X`
REVISION: `V001`
STATUS: `FAILED_CANDIDATE_PRESERVED; LEGAL_PARENT_IDENTIFIED`

## Legal parent identity

- Direct parent: `R31B V011`
- Parent source: `线上结果/R31B/V011/submission.asc`
- Parent source commit: `0a6b8f7cbcb1248601d76de3bc442a55141568fb`
- Parent source SHA-256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Preserved parent copy: `本地实验/UB-LIFETIME-SAFE-CHAMPION-X/V001/parent-submission.asc`
- Parent copy SHA-256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

The two parent paths were compared byte-for-byte before this note was written.

## Preserved failed candidate

- Candidate source: `本地实验/UB-LIFETIME-SAFE-CHAMPION-X/V001/submission.asc`
- Candidate source commit: `ccafe2c5c8254baac7e9c1169b1bb0e6a4778415`
- Candidate source SHA-256: `9b73bb5626b5e98be53faeb91eba024b2589bf0b8b5b46e36c48ec18e009f989`
- Candidate diff: `本地实验/UB-LIFETIME-SAFE-CHAMPION-X/V001/diff.patch`
- Correctness evidence commit: `c44e1a8933e11660ce78842231ffa2ef1069294a`

The candidate source is retained at its original evidence path and is not
overwritten by the parent bytes. Its exact-source build and correctness
artifacts remain under this V001 directory.

## Explicit restore decision

There is no separate mutable Candidate kernel path in this worktree. The
candidate exists only as the V001 evidence artifact. Therefore the compliant
restore operation is explicit parent-path selection using the two SHA-bound
parent files above. It does not require a source edit, a new Revision, or an
NPU rerun.

Do not run `git revert ccafe2c5c8254baac7e9c1169b1bb0e6a4778415`: that commit
adds the V001 candidate, parent copy, diff, and source hash, so reverting it
would remove the failed source evidence from the current tree. Do not use
`git reset`, `git clean`, or an in-place replacement of
`V001/submission.asc`.

No V002, performance mechanism, timing run, Online action, or shared-ledger
change is part of this restore trail.
