# N01 V021 Handoff

V021 source commit `c50d6f4a7f466fbde2c2b92eb544082cf3a1a0b5`, source SHA256 `d4f9994b9eb43e75b454143cbdb3f2b8a2222545d6ad4496fc6022c1682e5134`. The wide low-precision row path writes the squared tile to `residualLocal`, then reuses the no-longer-needed `xLocal` as the `ReduceSum` scratch tile.

- CANN 8.5 configure/build and device-code extraction: PASS, all exit codes `0`; AICore binary differs from V020.
- Correctness: PASS, 4/4 on d5, Runner exit `0`.
- Paired Local: completed, 4 x 21 on d5, Runner exit `0`; severe outliers in three cases, so `LOCAL_SCORE=NONE`.
- Runner Candidate labels `V001`/`V016` are generic slots; actual Candidate is V021. Parent is fixed R31B V011.
- d5 before and after Local: HBM `91%`, AICore `0%`; existing VLLMWorker PID `3355594` remained at `56354 MB`. No process was interrupted.
- Exact input identities, distributions, and raw evidence are in `RESULTS.md` and `results/`.

NEXT_ACTION: preserve and publish the V021 evidence commit only; no V022 was started in this task window.
