# STORE-EPILOGUE-W2-X V001 Correctness Runtime Path

## Diagnosis

- Candidate source SHA256: `48b9428dc2fc97c7c9d95f03ad8cec8e758c88e1197aa2328b1b1edebc018f88`
- Parent executable SHA256: `a55a4859dd22970216405826c7cb7f37fef4678c491979631505b0aac3f573a7`
- Candidate executable SHA256: `469522ba548c001e4ba223eb069d170fa0e73a8285f9f5aaaab696a5edd45328`
- Server Route checkout, dynamically located for `w2/m1/store-epilogue`: `/home/data4t2/lelinfeng/cann-w2-m1-store`, HEAD `5b632c4da540cb3d711d72c1c2b9843bae759eba`.
- Run 001 used an unset `LD_LIBRARY_PATH`; both executables reported `libgraph.so: not found` and additional Ascend libraries unresolved in `ldd`.
- The toolkit contains `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64/libgraph.so`.
- `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/lib64/libgraph.so` resolves to that same file.
- With `LD_LIBRARY_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64`, `ldd` resolved all dependencies for both Parent and Candidate executables.

## Harness Change

`run_correctness_v001.sh` now prepends the verified toolkit runtime library directory to `LD_LIBRARY_PATH`, preserves any existing entries, and exits before creating a result directory if the selected directory is absent. Candidate and Parent sources, executables, case matrix, and comparison logic are unchanged.

## Run 001 Preservation

Run 001 output remains at `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-001/` with driver log `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-001.log`. This change does not write to either path.

## Run 002 Command

```sh
cd /home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001
DEVICE_ID=5 RESULT_DIR=/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-002 \
  bash ./run_correctness_v001.sh \
  > /home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/correctness-run-002.log 2>&1
```

This run has not started at the time of this record.
