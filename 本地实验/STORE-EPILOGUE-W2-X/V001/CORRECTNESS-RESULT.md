# STORE-EPILOGUE-W2-X V001 Correctness Result

STAGE_STATUS=INCOMPLETE_ENVIRONMENT
HARNESS_STATUS=FAIL
HARNESS_RC=1
CORRECTNESS_END=2026-10-02T14:33:17+00:00

## Source and Executables

- Run context commit: `5b632c4da540cb3d711d72c1c2b9843bae759eba`
- Candidate source commit: `a1ebfc125ce215fc81ffbe04f582d5f8682e3294`
- Candidate `submission.asc` SHA256: `48b9428dc2fc97c7c9d95f03ad8cec8e758c88e1197aa2328b1b1edebc018f88`
- Direct Parent `parent.asc` SHA256: `59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839`
- Parent executable SHA256: `a55a4859dd22970216405826c7cb7f37fef4678c491979631505b0aac3f573a7`
- Candidate executable SHA256: `469522ba548c001e4ba223eb069d170fa0e73a8285f9f5aaaab696a5edd45328`

## Command and Result

```sh
cd /home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001
DEVICE_ID=5 RESULT_DIR=/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-001 \
  bash ./run_correctness_v001.sh \
  > /home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-001.log 2>&1
```

- Harness RC: `1`
- Case keys: `25`; unique keys: `25`; duplicates: `0`
- Parent RC: `127` in all 25 cases
- Candidate RC: `127` in all 25 cases
- Accepted cases: `0`; failed-to-launch cases: `25`
- All 50 per-case logs report `error while loading shared libraries: libgraph.so: cannot open shared object file`.
- The NPU executables did not start; no output tensors or `.bin` files were produced. Numeric correctness is unassessed.
- No retry, binary change, environment change, or timing was performed.

## Server Evidence

- Route branch checkout, located by `git worktree list`: `/home/data4t2/lelinfeng/cann-w2-m1-store`
- Route checkout HEAD: `5b632c4da540cb3d711d72c1c2b9843bae759eba`
- Experiment directory: `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001`
- Device 5 before run: HBM `64043/65536 MB`, free `1493 MB`, AICore `0%`; PID `89602` `VLLMWorker_TP` (`56678 MB`), PID `1819588` `python` (`3896 MB`)
- Filesystem available before run: `629 GB`
- Device 5 after run: HBM `64044/65536 MB`, free `1492 MB`, AICore `0%`; the same resident processes remained.
- Filesystem available after run: `629 GB`

## Logs

- Harness: `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-001.log`
- Matrix: `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-001/correctness-summary.tsv`
- Parent and Candidate case logs: `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-001/`
- First Parent log: `parent_r1_d1024_t0.log`
- First Candidate log: `candidate_r1_d1024_t0.log`
