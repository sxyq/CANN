# CANN W4 second-round sources-only export

This is a source and Markdown export only. `Official-equivalent=NO` for every entry. No Online submission was executed or authorized by this package.

## Export

- Candidate count: 4 distinct `submission.asc` sources.
- Exported directories: `MODE-V074`, `ROW-V058`, `ROW-V062`, `SYNC-V006`.
- Requested export priority: ROW V058, ROW V062, MODE V074, SYNC V006.
- Review recommendation (not an Online decision): MODE V074, ROW V058, SYNC V006, ROW V062. MODE has the closest positive but noisy retest signal; ROW V058 is less negative than ROW V062; SYNC is explicitly rejected as noisy; ROW V062 is the weakest local signal.
- All candidate Local values are route-local measurements with different shapes, formulas, and cohorts. They are not Official scores.
- The prior-round same-source candidates V047, V096, and V002 were not copied.

## Candidates

| Export directory | Route / Revision | Source SHA-256 | Local status | Online |
|---|---|---|---|---|
| `ROW-V058` | ROW-SCALE-HOIST-X / V058 | `6c4a11c570226ec17821063c33c6e5f48f1f9a0ec7c9f5b825c38c766ef7b20d` | `MEASUREMENT_BLOCKED` | NO |
| `ROW-V062` | ROW-SCALE-HOIST-X / V062 | `1dd6fb4b2a29d752b0d8a190cc5c8e5310a7725a8aee05d708bf0a0f43955996` | `MEASUREMENT_BLOCKED` | NO |
| `MODE-V074` | R-W4-4 / V074 | `d3a471ccd4e82bb1244ecb0b991f693d52e5e674cd2def36a19f4b62a34d8592` | `NOISY/NEEDS_ONE_MORE_LOCAL` | NO |
| `SYNC-V006` | SYNC-BARRIER-ELISION-X / V006 | `acff22e50cc4aa2ddea833e6ef8da6625800740ac71cc77841e63cb11c6d6581` | `LOCAL_RETEST_REJECTED_NOISY` | NO |

## Not exported

- STAGING: `NO_NEW_CANDIDATE`. V034 remains the only distinct source identity; V014 and V018 share SHA `75131108d32bc587c3c289d53ded04a15c5224417c675cb37a46df032e104d33`. No staging submission was copied.
- ALIGNED: `NO_NEW_CANDIDATE`. No distinct Candidate exists; V001 is byte-identical to the Parent SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`. No aligned submission was copied.
- Previous-round ROW V047, MODE V096, and SYNC V002 were not used as second-round candidates.

## Package boundary

The ZIP contains only this root README plus one `submission.asc` and one candidate README per exported directory. It contains no logs, JSON, CSV, TSV, build output, Git objects, runner, score script, or automatic Online action. The `submission.asc` files include only standard/toolchain headers outside this package.
