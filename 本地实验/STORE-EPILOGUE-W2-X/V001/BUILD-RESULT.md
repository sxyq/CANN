# STORE-EPILOGUE-W2-X V001 Build Result

STATUS=PASS
BUILD_START=2026-10-02T14:26:36+00:00
BUILD_END=2026-10-02T14:27:54+00:00

## Source

- Run context commit: `5b632c4da540cb3d711d72c1c2b9843bae759eba`
- Candidate source commit: `a1ebfc125ce215fc81ffbe04f582d5f8682e3294`
- Candidate `submission.asc` SHA256: `48b9428dc2fc97c7c9d95f03ad8cec8e758c88e1197aa2328b1b1edebc018f88`
- Direct Parent `parent.asc` SHA256: `59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839`
- Build script SHA256: `8e224295a88927894c8750eb1d0bf8b371920f529d98329b421208f5dc580a0e`

## Command and Result

```sh
cd /home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001
mkdir -p ./logs
bash ./build_v001.sh > ./logs/build-driver.log 2>&1
```

- Build driver RC: `0`
- CMake configure RC: `0`
- Candidate object RC: `0`
- Parent correctness executable RC: `0`
- Candidate correctness executable RC: `0`
- Parent executable SHA256: `a55a4859dd22970216405826c7cb7f37fef4678c491979631505b0aac3f573a7`
- Candidate executable SHA256: `469522ba548c001e4ba223eb069d170fa0e73a8285f9f5aaaab696a5edd45328`

## Server

- Host: `hwnput3`
- Route branch checkout, located by `git worktree list`: `/home/data4t2/lelinfeng/cann-w2-m1-store`
- Route checkout HEAD: `5b632c4da540cb3d711d72c1c2b9843bae759eba`
- Experiment directory: `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001`
- Toolkit: `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`
- SoC: `Ascend910B3`; `npu-smi` version: `25.0.rc1.1`
- Device 5 before Build: HBM `64044/65536 MB`, free `1492 MB`, AICore `3%`
- Device 5 resident processes: PID `89602` `VLLMWorker_TP` (`56678 MB`), PID `1819588` `python` (`3896 MB`)
- Filesystem available before Build: `629 GB`

## Logs

- Driver: `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/build-driver.log`
- Configure: `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/cmake-configure.log`
- Candidate object: `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/build-submission-object.log`
- Parent executable: `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/build-parent-correctness.log`
- Candidate executable: `/home/data4t2/lelinfeng/cann/STORE-EPILOGUE-W2-X/V001/logs/build-candidate-correctness.log`
