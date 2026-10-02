# HBH-10 V001 Final Qualification Bootstrap Provenance

The first two launches of the final d4 qualification did not enter the NPU
runner. They stopped while sourcing the CANN environment under `set -u`:

| Attempt | Preserved log | Support-only startup error | NPU qualification entered |
|---|---|---|---|
| 002 | `qualification-final-002.log` | `set_env.sh: line 1: LD_LIBRARY_PATH: unbound variable` | NO |
| 002-fixed | `qualification-final-002-fixed.log` | `set_env.sh: line 5: PYTHONPATH: unbound variable` | NO |

The final run used the existing timing-build initialization convention,
allowing empty `LD_LIBRARY_PATH` and `PYTHONPATH` before sourcing
`set_env.sh`. It used the unchanged V001 executable, source identities,
device lease, shape, warmups, samples, blocks, and qualification thresholds.
No Candidate Kernel, timing harness source, or protocol parameter changed.
