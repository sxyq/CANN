# SYNC-BARRIER-ELISION-X V060 Device Release

- Device: 3.
- Revision: V060 Parent/Candidate qualification and Local capture complete.
- Captured: both 31-pair interleaved blocks, all 62 raw pairs retained; result is `LOCAL_REJECTED_NOISY` with numeric score `+8.373702%` and paired median delta `-1.2100 us`.
- Released: `2026-10-08T11:34:20Z`, after the post-capture snapshot.
- Post-capture state: HBM `3,939/65,536 MB` used (`61,597 MB` free), AICore `1%`; existing Python PID `2975184` remained present and untouched; no V060 runner process remained.
- Evidence: `logs/device3-postcapture-release.log`, `local-result.json`, and the two interleaved raw logs.
- Any future device use requires a fresh exclusive assignment.
